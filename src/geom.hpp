#include <Eigen/Core>
#include <Eigen/Geometry>

#include <ujson/ujson.hpp>

using Eigen::Matrix;
using Eigen::Vector3f;
using Eigen::Vector3i;
using Eigen::Vector4f;
using Eigen::Matrix3f;
using Eigen::Matrix4f;

using namespace std;


typedef float Number;
using Vector3 = Matrix<Number, 3, 1>;
using Matrix3 = Matrix<Number, 3, 3>;

const float EPSILON = 1e-5;

static Number getRandom() {
	return (Number)rand()/(RAND_MAX+1);
}

static Number getRandomRange(Number from, Number to) {
	return from + (to - from)*getRandom();
}


Vector3 getOrthogonalVector(Vector3 v) {
	return v.x() != 0 || v.y() != 0 ?
			Vector3(v.y(), -v.x(), 0) :
			Vector3(v.z(), 0, -v.x());
}

Vector3 getVectorRotatedAroundAxis(Vector3 v, Number angle, Vector3 axis) {
	Number q0 = (Number)cos(angle*0.5);
	Number qsin = (Number)sin(angle*0.5);
	Number q1 = qsin*axis.x();
	Number q2 = qsin*axis.y();
	Number q3 = qsin*axis.z();
	Number q0s = q0*q0;
	Number q1s = q1*q1;
	Number q2s = q2*q2;
	Number q3s = q3*q3;
	Matrix3 rotation;
	rotation <<
		q0s + q1s - q2s - q3s,         2*(q1*q2 - q0*q3),         2*(q1*q3 + q0*q2),
		    2*(q2*q1 + q0*q3),   (q0s - q1s + q2s - q3s),         2*(q2*q3 - q0*q1),
			2*(q3*q1 - q0*q2),         2*(q3*q2 + q0*q1),   (q0s - q1s - q2s + q3s);
	return rotation * v;
}

Vector3 getVectorFromAzimuthZenithAndNormal(Number phi, Number theta, Vector3 normal) {
	Vector3 x = getOrthogonalVector(normal);
	Vector3 y = getVectorRotatedAroundAxis(x, phi, normal);
	Vector3 perp = normal.cross(y);
	return getVectorRotatedAroundAxis(normal, theta, perp).normalized();
}




struct IMaterial
{
	virtual ~IMaterial() {}
	virtual Vector3 getEmission() const = 0;
	virtual Vector3 getReflectance(Number inPhi, Number inTheta, Number outPhi, Number outTheta, Vector3 pos) const = 0;
	virtual Vector3 getDirection(Vector3 &pos, Vector3 &normal) const = 0;
};


struct Material
{
	Vector3 emission;
	Vector3 albedo;
	
	Material() :
		albedo(Vector3(0.2, 0.5, 0.8)),
		emission(Vector3(0, 0, 0))
		{}
	
	Vector3 getEmission() const {
		return emission;
	}
	
	Vector3 getReflectance(Number inPhi, Number inTheta, Number outPhi, Number outTheta, Vector3 pos) const {
		return albedo*1/M_PI*cos(outTheta);
	}
	
	Vector3 getDirection(Vector3 &pos, Vector3 &normal) const {
		Number phi = getRandomRange(0, 2*M_PI);
		Number rho = acos(sqrt(getRandom()));
		Number theta = M_PI/2 - rho;
		return getVectorFromAzimuthZenithAndNormal(phi, theta, normal);
	}
};


struct Plane {
	Number a;
	Number b;
	Number c;
	Number d;
	
	static Vector3 intersectionPoint(Plane a, Plane b, Plane c) {
		Vector3 an = a.getNormal();
		Vector3 bn = b.getNormal();
		Vector3 cn = c.getNormal();
		return (
				-a.d*(bn.cross(cn))
				-b.d*(cn.cross(an))
				-c.d*(an.cross(bn))
				)
				/ an.dot(bn.cross(cn));
	}
	
	void normalize() {
		Number len = sqrt(a*a+b*b+c*c);
		a /= len;
		b /= len;
		c /= len;
		d /= len;
	}
	
	Vector3 getNormal() {
		return Vector3(a, b, c);
	}
	
	Number side(Vector3 &p) {
		return a*p.x()+b*p.y()+c*p.z()+d;
	}
	
	friend ostream& operator<<(ostream& os, const Plane& p);
};
ostream& operator<<(ostream& os, const Plane& p)
{
	os << p.a << ' ' << p.b << ' ' << p.c << ' ' << p.d;
	return os;
}


struct Rect3 {
	Vector3 tl;
	Vector3 tr;
	Vector3 bl;
	Vector3 br;
	Vector3 pointOnSurface(Number x, Number y) {
		Number wtl = (1-x)*(1-y);
		Number wtr = x*(1-y);
		Number wbl = (1-x)*y;
		Number wbr = x*y;
		return tl*wtl + tr*wtr + bl*wbl + br*wbr;
	}
	
	friend ostream& operator<<(ostream& os, const Rect3& p);
};
ostream& operator<<(ostream& os, const Rect3& r)
{
	os << '(' << r.tl.x() << ',' << r.tl.y() << ',' << r.tl.z() << " : " << r.tr.x() << ',' << r.tr.y() << ',' << r.tr.z() << ')' << endl;
	return os;
};

struct Frustum {
	Plane left;
	Plane right;
	Plane bottom;
	Plane top;
	Plane near;
	Plane far;
	Rect3 nearRect;
	
	void update(Matrix<Number, 4, 4> &matrix) {
		updatePlane(matrix, left,	 1, 0, 3);
		updatePlane(matrix, right,	-1, 0, 3);
		updatePlane(matrix, bottom,	 1, 1, 3);
		updatePlane(matrix, top,	-1, 1, 3);
		updatePlane(matrix, near,	 1, 2, 3);
		updatePlane(matrix, far,	-1, 2, 3);
		updateNearCorners();
	}
	
	Vector3 pointOnNearPlane(Number x, Number y) {
		return nearRect.pointOnSurface(x, y);
	}
	
	protected:
	void updatePlane(Matrix<Number, 4, 4> &matrix, Plane &plane, Number signColA, int colA, int colB) {
		plane.a = signColA*matrix(colA, 0) + matrix(colB, 0);
		plane.b = signColA*matrix(colA, 1) + matrix(colB, 1);
		plane.c = signColA*matrix(colA, 2) + matrix(colB, 2);
		plane.d = signColA*matrix(colA, 3) + matrix(colB, 3);
		plane.normalize();
	}
	
	void updateNearCorners() {
		nearRect.tl = Plane::intersectionPoint(near, top, left);
		nearRect.tr = Plane::intersectionPoint(near, top, right);
		nearRect.bl = Plane::intersectionPoint(near, bottom, left);
		nearRect.br = Plane::intersectionPoint(near, bottom, right);
	}
	
	friend ostream& operator<<(ostream& os, const Frustum& f);
};
ostream& operator<<(ostream& os, const Frustum& f)
{
	os << "Frustum" << endl;
	os << "Left:   " << f.left << endl;
	os << "Right:  " << f.right << endl;
	os << "Bottom: " << f.bottom << endl;
	os << "Top:    " << f.top << endl;
	os << "Near:   " << f.near << endl;
	os << "Far:    " << f.far << endl;
	return os;
}



struct Camera {
	
	Matrix4f view;
	Matrix4f projection;
	
	Matrix4f transform;
	
	Camera() {
		lookAt(Vector3(0, 0, 0), Vector3(0, 0, -1), Vector3(0, 1, 0));
	}
	
	void update() {
		projection = perspectiveProjection();
		transform = projection*view;
	}
	
	Matrix4f perspectiveProjection(Number fov = 40, Number aspect = 1, Number near = 1, Number far = 100) {
		Number y2 = near * tan(fov * M_PI / 360);
		Number y1 = -y2;
		Number x1 = y1 * aspect;
		Number x2 = y2 * aspect;
		
		Number a  = 2 * near / (x2 - x1);
		Number b  = 2 * near / (y2 - y1);
		Number c  = (x2 + x1) / (x2 - x1);
		Number d  = (y2 + y1) / (y2 - y1);
		Number q  = -(far + near) / (far - near);
		Number qn = -2 * (far * near) / (far - near);
		
		Matrix4f r;
		r <<
			a, 0, 0, 0,
			0, b, 0, 0,
			c, d, q, -1,
			0, 0, qn, 0
		;
		return r;
	}
	
	// Right-handed
	void lookAt(Vector3 eye, Vector3 target, Vector3 up) {
		Vector3 zAxis = (eye - target).normalized();
		Vector3 xAxis = up.cross(zAxis).normalized();
		Vector3 yAxis = zAxis.cross(xAxis);
		/*
		view <<      xAxis.x(),       yAxis.x(),       zAxis.x(), 0,
			         xAxis.y(),       yAxis.y(),       zAxis.y(), 0,
			         xAxis.z(),       yAxis.z(),       zAxis.z(), 0,
		       -xAxis.dot(eye), -yAxis.dot(eye), -zAxis.dot(eye), 1;
		*/
		
		Vector3 f = (target - eye).normalized();
		Vector3 un = up.normalized();
		Vector3 s = f.cross(un);
		Vector3 newu = s.cross(f);
		
		Matrix4f M;
		
		M << s.x(), s.y(), s.z(), 0,
		     newu.x(), newu.y(), newu.z(), 0,
			 -f.x(), -f.y(), -f.z(), 0,
			 0, 0, 0, 1;
			 
		Matrix4f T;
		
		T << 1, 0, 0, -eye.x(),
		     0, 1, 0, -eye.y(),
		     0, 0, 1, -eye.z(),
			 0, 0, 0, 1;
			 
		view = M*T;
		
		update();
	}
	
};


struct Intersection {
	Number t;
	Vector3 pos;
	Vector3 normal;
	Material *material;
	Intersection(Number t = INFINITY) : t(t), material(NULL) {};
};

struct Ray {
	Vector3 pos;
	Vector3 dir;
	Number weight;
	
	Intersection nearest;
	
	bool done;
	Vector3 color;
};

struct Sphere {
	Vector3 pos;
	Number radius;
	
	Material material;
	
	/*
	Vector3 getEmission() const {
		return Vector3(1, 1, 1);
	}
	
	Vector3 getReflectance(Number inPhi, Number inTheta, Number outPhi, Number outTheta, Vector3 pos) const {
		return albedo*1/M_PI*cos(outTheta);
	}
	
	Vector3 getDirection(Vector3 &pos, Vector3 &normal) const {
		Number phi = getRandomRange(0, 2*M_PI);
		Number rho = acos(sqrt(getRandom()));
		Number theta = M_PI/2 - rho;
		return getVectorFromAzimuthZenithAndNormal(phi, theta, normal);
	}
	*/
	
	Intersection intersects(Ray &r) const {
		/*
		Vector3 t = pos - r.pos;
		Number tc = r.dir.dot(t);
		if (tc < 0) return Intersection(-1);
		
		t = r.pos + r.dir*tc;
		Number dsq = (pos - t).squaredNorm();
		Number rsq = radius * radius;
		if (dsq > rsq) return Intersection(-2);
		
		t = r.dir * (tc - sqrt(rsq - dsq));
		
		Intersection inter = Intersection(tc);
		inter.pos = r.pos + t;
		return inter;
		*/
		
		Vector3 d = r.pos - pos;
		Number a = r.dir.squaredNorm();
		Number b = 2*r.dir.dot(d);
		Number c = d.squaredNorm() - radius*radius;
		Number radicand = b*b - 4*a*c;
		
		if (radicand < 0) return Intersection(-1);
		
		Number div = 1/(2*a);
		Number radicandRoot = sqrt(radicand);
		Number u1 = (-b + radicandRoot)*div;
		Number u2 = (-b - radicandRoot)*div;
		
		if (u1 < EPSILON && u2 < EPSILON) return Intersection(-2);
		
		Number u = u2 < EPSILON ? u1 : min(u1, u2);
		
		Intersection inter = Intersection(u);
		inter.pos = r.pos + u * r.dir;
		inter.normal = (inter.pos - pos).normalized();
		return inter;
	}
};


struct Scene {
	thrust::host_vector<Sphere> spheres;
	void addSphere(Number x, Number y, Number z, Number r) {
		Sphere s = Sphere();
		s.pos = Vector3(x, y, z);
		s.radius = r;
		spheres.push_back(s);
	}
};
/*
 * geometry.h
 */

#ifndef __GEOMETRY_H
#define __GEOMETRY_H

#define PI       3.14159265358979
#define TWO_PI   6.28318530717959
#define HALF_PI  1.570796326794895

#define DEGREES_60 1.0471975511965976
#define DEGREES_75 1.309

#define LINE_TO_POINT_HDG_THRESHOLD   DEGREES_75

#define TO_DEGREES(a)  ((a) * (180.0/PI))
#define TO_RADIANS(a)  ((a) * (PI/180.0))


class Point {
 public:
 Point(float _x = 0, float _y = 0)
     : x(_x), y(_y) { };
    ~Point() {};
    float getDistance( float _x, float _y );
    float getDirection( float _x, float _y );
    float getDirection( const Point & p );
    void operator+=(Point & p) { x += p.x; y += p.y;};

    // fuzzy compare
    bool operator==(const Point &other) const;
    bool operator!=(const Point &other) const;

    float x, y;
};

class Pose : public Point {
 public:
 Pose(float _x = 0, float _y = 0, float _theta = 0)
     : Point(_x, _y), theta(_theta) { };
    ~Pose() {};

    void local_to_global(Point & robot_local, Point & global_point);
    void global_to_local(Point & global_point, Point & robot_local);

    float theta;
};

// Struttura contenente informazioni sull'intersezione di due oggetti
class IntersectionData
{
	public:
		// Gli oggetti considerati hanno una porzione di area o perimetro in comune?
		bool areasIntersect() const;

		// Punti di intersezione tra perimetri, curve e/o segmenti degli oggetti considerati
		unsigned int intersectionCount() const; // numero di punti di intersezione (0 se nessuno, UINT_MAX se infiniti)
		const Point &intersectionPoint(unsigned int i) const; // restituisce punto di intersezione i-esimo

		bool operator==(const IntersectionData &other) const;
		bool operator!=(const IntersectionData &other) const;

		// Metodi utilizzati per costruire oggetti di tipo IntersectionData
		// Tutti restituiscono un riferimento a *this
		IntersectionData();
		IntersectionData &setAreasIntersectFlag();
		IntersectionData &setInfiniteIntersectionPoints();
		IntersectionData &addIntersectionPoint(const Point &p);
		IntersectionData &merge(const IntersectionData &other);

		constexpr static unsigned int MAX_INTERSECTIONS = 8;

	private:
		bool m_areasIntersect;
		unsigned int m_numPoints;
		Point m_points[MAX_INTERSECTIONS];
};

// Circonferenza
class Circle
{
	public:
		Circle() = delete;

		// Circonferenza di dato centro e raggio
		Circle(const Point &center, float radius);
		const Point &center() const;
		float radius() const;

		IntersectionData intersect(const Circle &c) const;

		bool containsPoint(const Point &point) const;

	private:
		Point m_center;
		float m_radius;
};

// Retta
class Line
{
	friend class Segment;

	public:
		Line() = delete;

		// Retta passante per due punti
		Line(const Point &pointA, const Point &pointB);
		const Point &pointA() const;
		const Point &pointB() const;

		IntersectionData intersect(const Circle &c) const;
		IntersectionData intersect(const Line &l) const;

		Point projectPoint(const Point &pnt) const;
		float pointDistance(const Point &pnt) const;

	private:
		Point m_pointA, m_pointB;

		bool calcCircleIntersectionDistances(const Circle &cir, float *ps0, float *ps1) const;
};

// Segmento
class Segment
{
	public:
		Segment() = delete;

		// Segmento tra due punti
		Segment(const Point &pointA, const Point &pointB);
		const Point &pointA() const;
		const Point &pointB() const;

		IntersectionData intersect(const Circle &c) const;
		IntersectionData intersect(const Line &l) const;
		IntersectionData intersect(const Segment &s) const;

		bool containsProjectedPoint(const Point &point) const;

		// Calcola l'eventuale punto di intersezione più vicino ad a
		bool intersectAndReturnNearestPoint(const Circle &cir, Point *pContact) const;

	private:
		Point m_pointA, m_pointB;
};

// Poligono convesso
class Polygon
{
	public:
		Polygon() = delete;

		// Poligono convesso delimitato da una spezzata chiusa
		Polygon(const Point vertexArray[], unsigned int vertexCount);
		const Point &vertexAtIndex(unsigned int index) const;
		const Point *vertexArray() const;
		unsigned int vertexCount() const;

		IntersectionData intersect(const Circle &c) const;
		IntersectionData intersect(const Line &l) const;
		IntersectionData intersect(const Segment &s) const;
		IntersectionData intersect(const Polygon &s) const;

		bool containsPoint(const Point &point) const;

		constexpr static unsigned int MAX_VERTICES = 4;

	private:
		Segment segmentAtIndex(unsigned int index) const;

		unsigned int m_vertexCount;
		Point m_vertexArray[MAX_VERTICES];
};

// Capsula
class Capsule
{
	public:
		Capsule() = delete;

		// Luogo dei punti la cui distanza dal segmento AB è minore o uguale a radius
		Capsule(const Point &pointA, const Point &pointB, float radius);
		const Point &pointA() const;
		const Point &pointB() const;
		float radius() const;

		// Interseca due capsule e restituisce true se hanno almeno un
		// punto in comune
		bool intersect(const Capsule &other) const;

	private:
		Circle m_estrA, m_estrB; // cerchi alle estremità
		Polygon m_body; // rettangolo centrale
};

// Differenza tra due angoli, compresa tra -180 e 180
#define ANG_DIFF(ANG_A, ANG_B)	( fmod((ANG_B) - (ANG_A) + 360*2 + 180, 360) - 180 )


float normalize_angle(float a);
float normalize_angle_degrees(float a);
float normalize_angle_degrees_360(float a);
float hypot(float a, float b);
float distance(const Point &a, const Point &b);
float distance2(const Point &a, const Point &b);

#endif

#pragma once

#include <cmath>
#include <array>

namespace fib {
	
constexpr float pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148086513282306647093844609550582231725359408128f;

template <class T>
class point2d
{
private:
	T x_{}, y_{};
public:
	T const& x() const { return x_; }
	T const& y() const { return y_; }
	T& x() { return x_; }
	T& y() { return y_; }

	constexpr point2d(T x, T y) : x_(x), y_(y) {}
	constexpr point2d() = default;
};

template <class T>
class vector2d
{
private:
	using point_t = point2d<T>;

	point_t p1_{}, p2_{};
public:
	vector2d(point_t const& p1, point_t const& p2) : p1_(p1), p2_(p2) {}

	point_t const& p1() const { return p1_; }
	point_t const& p2() const { return p2_; }
	point_t& p1() { return p1_; }
	point_t& p2() { return p2_; }
};

template <int Angle /* degrees */, class T, int Rows, int Cols>
class rotation_matrix;

template <int Angle>
class rotation_matrix<Angle, float, 2, 2>
{
private:
	using self_t = rotation_matrix<Angle, float, 2, 2>;

	std::array<float, 2 * 2> storage_{ 0, };

public:
	rotation_matrix();
	rotation_matrix(self_t const&) = default;
	rotation_matrix(self_t&&) = default;

	constexpr std::size_t rows() const { return 2; }
	constexpr std::size_t cols() const { return 2; }
	float const& operator()(std::size_t i, std::size_t j) const { assert(i >= 0 && i < rows() && j >= 0 && j < cols()); return this->storage_[i * cols() + j]; }

	// setters
	float& operator()(std::size_t i, std::size_t j) { assert(i >= 0 && i < rows() && j >= 0 && j < cols()); return this->storage_[i * cols() + j]; }

	// operations
	point2d<float> operator*(point2d<float> const& point) const;
	vector2d<float> operator*(vector2d<float> const& v) const;
};

template <int Angle>
rotation_matrix<Angle, float, 2, 2>::rotation_matrix<Angle, float, 2, 2>()
{
	auto angle = float(Angle) * pi / 180;
	storage_[0] =  std::cos(angle);
	storage_[1] = -std::sin(angle);
	storage_[2] =  std::sin(angle);
	storage_[3] =  std::cos(angle);
};

template <int Angle>
point2d<float> rotation_matrix<Angle, float, 2, 2>::operator*(point2d<float> const& point) const
{
	float x = storage_[0] * point.x() + storage_[1] * point.y();
	float y = storage_[2] * point.x() + storage_[3] * point.y();
	return point2d<float>(x, y);
};

template <int Angle>
vector2d<float> rotation_matrix<Angle, float, 2, 2>::operator*(vector2d<float> const& v) const
{
	auto p2 = v.p2();

	// bring to origin
	p2.x() = p2.x() - v.p1().x();
	p2.y() = p2.y() - v.p1().y();

	p2 = this->operator*(p2);
	// bring back
	p2.x() += v.p1().x();
	p2.y() += v.p1().y();

	return vector2d<float>(v.p1(), p2);
};

/*
 *  row-major storage static rotation matrix
 */
template <>
class rotation_matrix<90, float, 2, 2>
{
private:
	using self_t = rotation_matrix<90, float, 2, 2>;

	std::array<float, 2 * 2> storage_{ 0, };

public:
	static float cos;
	static float sin;

	// ctors
	rotation_matrix();
	rotation_matrix(self_t const&) = default;
	rotation_matrix(self_t&&) = default;

	// getters
	constexpr std::size_t rows() const { return 2; }
	constexpr std::size_t cols() const { return 2; }
	float const& operator()(std::size_t i, std::size_t j) const { assert(i >= 0 && i < rows() && j >= 0 && j < cols()); return this->storage_[i * cols() + j]; }

	// setters
	float& operator()(std::size_t i, std::size_t j) { assert(i >= 0 && i < rows() && j >= 0 && j < cols()); return this->storage_[i * cols() + j]; }

	// operations
	point2d<float> operator*(point2d<float> const& point) const;
	vector2d<float> operator*(vector2d<float> const& v) const;
	self_t operator*(self_t const& other);
};

template <int Angle>
using static_matrix_2f = rotation_matrix<Angle, float, 2, 2>;

using static_matrix_2f_90d = static_matrix_2f<90>;

float rotation_matrix<90, float, 2, 2>::cos = 0.f;

float rotation_matrix<90, float, 2, 2>::sin = 1.f;

rotation_matrix<90, float, 2, 2>::rotation_matrix<90, float, 2, 2>()
{
	storage_[0] =   self_t::cos;
	storage_[1] = - self_t::sin;
	storage_[2] =   self_t::sin;
	storage_[3] =   self_t::cos;
};

point2d<float> rotation_matrix<90, float, 2, 2>::operator*(point2d<float> const& point) const
{
	float x = storage_[0] * point.x() + storage_[1] * point.y();
	float y = storage_[2] * point.x() + storage_[3] * point.y();
	return point2d<float>(x, y);
};

vector2d<float>  rotation_matrix<90, float, 2, 2>::operator*(vector2d<float> const& v) const
{
	auto p2 = v.p2();

	// bring to origin
	p2.x() = p2.x() - v.p1().x();
	p2.y() = p2.y() - v.p1().y();

	p2 = this->operator*(p2);
	// bring back
	p2.x() += v.p1().x();
	p2.y() += v.p1().y();

	return vector2d<float>(v.p1(), p2);
};

rotation_matrix<90, float, 2, 2> rotation_matrix<90, float, 2, 2>::operator*(self_t const& other)
{
	self_t result{};

	result(0, 0) = this->operator()(0, 0) * other(0, 0) + this->operator()(0, 1) * other(1, 0);
	result(0, 1) = this->operator()(0, 0) * other(0, 1) + this->operator()(0, 1) * other(1, 1);
	result(1, 0) = this->operator()(1, 0) * other(0, 0) + this->operator()(1, 1) * other(1, 0);
	result(1, 1) = this->operator()(1, 0) * other(0, 1) + this->operator()(1, 1) * other(1, 1);

	return result;
};


template <class T, int Rows, int Cols>
class scale_translate_matrix;

template <class T, int Rows, int Cols>
class circular_scale_iterator;

template <>
class scale_translate_matrix<float, 2, 3>
{
private:
	using self_t = scale_translate_matrix<float, 2, 3>;

	// give iterator access
	friend class circular_scale_iterator<float, 2, 3>;

	std::array<float, 2 * 3> storage_ = { 1, 0, 0, 0, 1, 0 };

	float const& operator()(std::size_t i, std::size_t j) const { return storage_[i * cols() + j]; }
	float& operator()(std::size_t i, std::size_t j) { return storage_[i * cols() + j]; }
public:
	// ctors
	scale_translate_matrix() = default;
	scale_translate_matrix(float kx, float ky) : storage_{ kx, 0, 0, 0, ky, 0} {}
	scale_translate_matrix(float kx, float ky, float tx, float ty) : storage_{ kx, 0, tx, 0, ky, ty } {}

	// getters
	float const& scale_x() const { return storage_[0]; }
	float const& scale_y() const { return storage_[cols() + 1]; }
	float const& translate_x() const { return storage_[cols() - 1]; }
	float const& translate_y() const { return storage_[cols() * rows() - 1]; }


	constexpr std::size_t rows() const { return 2; }
	constexpr std::size_t cols() const { return 3; }

	// setters
	void scale_x(float kx) { storage_[0] = kx; }
	void scale_y(float ky) { storage_[cols() + 1] = ky; }
	void translate_x(float kx) { storage_[cols() - 1] = kx; }
	void translate_y(float ky) { storage_[cols() * rows() - 1] = ky; }

	// operations
	point2d<float> operator*(point2d<float> const& point) const;
	vector2d<float> operator*(vector2d<float> const& v) const;
};

using scale_translate_matrix_2f = scale_translate_matrix<float, 2, 3>;

point2d<float> scale_translate_matrix<float, 2, 3>::operator*(point2d<float> const& point) const
{
	float x = scale_x() * point.x() + translate_x();
	float y = scale_y() * point.y() + translate_y();
	return point2d<float>(x, y);
};

vector2d<float> scale_translate_matrix<float, 2, 3>::operator*(vector2d<float> const& v) const
{
	auto p2 = v.p2();

	// bring to origin
	p2.x() = p2.x() - v.p1().x();
	p2.y() = p2.y() - v.p1().y();

	p2 = this->operator*(p2);

	// bring back
	p2.x() += v.p1().x();
	p2.y() += v.p1().y();

	// origin
	auto p1 = std::decay_t<decltype(v.p1())>(0.f, 0.f);
	p1 = this->operator*(p1);

	// bring back
	p1.x() += v.p1().x();
	p1.y() += v.p1().y();

	return vector2d<float>(p1, p2);
};

template <class T, int Rows, int Cols>
class circular_scale_iterator;

template <>
class circular_scale_iterator<float, 2, 3>
{
private:
	using self_t = circular_scale_iterator<float, 2, 3>;
	using value_t = std::tuple<int, int, int, int, int>; /* scale_x, scale_y, sign, translate_x, translate_y */

	static constexpr std::array<value_t, 2 * 2> transformations
	{
		value_t{ 0, 0, -1, 0, 2 },
		value_t{ 1, 1, -1, 1, 2 },
		value_t{ 0, 0,  1, 0, 2 },
		value_t{ 1, 1,  1, 1, 2 }
	};

	std::size_t i = 0;

public:
	self_t& operator++() { i = (i + 1) % self_t::transformations.size(); return *this; }
	self_t const* operator->() const { return this; }

	self_t operator+(std::size_t n) const
	{ 
		self_t res; 
		res.i = (i + n) % self_t::transformations.size(); 
		return res; 
	}

	scale_translate_matrix_2f scale_translate(float k, float d) const
	{ 
		const auto& [sx, sy, sign, tx, ty] = self_t::transformations[i];

		scale_translate_matrix_2f result{};
		result(sx, sy) = k;
		result(tx, ty) = sign * d;

		return result;
	}
};

using circular_scale_iterator_2f = circular_scale_iterator<float, 2, 3>;

}
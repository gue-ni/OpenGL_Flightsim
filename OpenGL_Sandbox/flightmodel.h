#pragma once

#include "phi.h"
#include "gfx.h"

#include <cmath>
#include <vector>
#include <algorithm>

constexpr float log_intervall = 1.5f;

struct ValueTupel {
	float alpha;
	float cl;
	float cd;
};

float max(float a, float b)
{
	return a > b ? a : b;
}

float min(float a, float b)
{
	return a < b ? a : b;
}

float clamp(float v, float lo, float hi)
{
	return min(max(v, lo), hi);
}

std::vector<ValueTupel> NACA_0015 = {
 { -11.000,  -0.8022,   0.07748 }, 
 { -10.750,  -0.8442,   0.07035 }, 
 { -10.500,  -0.8851,   0.06475 }, 
 { -10.250,  -0.9176,   0.05969 }, 
 { -10.000,  -0.9434,   0.05517 }, 
 {  -9.750,  -0.9165,   0.05358 }, 
 {  -9.500,  -0.9248,   0.05030 }, 
 {  -9.250,  -0.9332,   0.04694 }, 
 {  -9.000,  -0.9132,   0.04547 }, 
 {  -8.750,  -0.9076,   0.04310 }, 
 {  -8.500,  -0.8997,   0.04122 }, 
 {  -8.250,  -0.8818,   0.03986 }, 
 {  -8.000,  -0.8680,   0.03838 }, 
 {  -7.750,  -0.8545,   0.03697 }, 
 {  -7.500,  -0.8402,   0.03572 }, 
 {  -7.250,  -0.8248,   0.03461 }, 
 {  -7.000,  -0.8089,   0.03359 }, 
 {  -6.750,  -0.7934,   0.03263 }, 
 {  -6.500,  -0.7795,   0.03164 }, 
 {  -6.250,  -0.7622,   0.03086 }, 
 {  -6.000,  -0.7431,   0.03032 }, 
 {  -5.750,  -0.7273,   0.02963 }, 
 {  -5.500,  -0.7103,   0.02904 }, 
 {  -5.250,  -0.6910,   0.02869 }, 
 {  -5.000,  -0.6765,   0.02808 }, 
 {  -4.750,  -0.6558,   0.02790 }, 
 {  -4.500,  -0.6380,   0.02761 }, 
 {  -4.250,  -0.6209,   0.02729 }, 
 {  -4.000,  -0.6011,   0.02725 }, 
 {  -3.750,  -0.5860,   0.02691 }, 
 {  -3.500,  -0.5652,   0.02701 }, 
 {  -3.250,  -0.5463,   0.02705 }, 
 {  -3.000,  -0.5299,   0.02694 }, 
 {  -2.750,  -0.5069,   0.02726 }, 
 {  -2.500,  -0.4886,   0.02733 }, 
 {  -2.250,  -0.4624,   0.02772 }, 
 {  -2.000,  -0.4322,   0.02817 }, 
 {  -1.750,  -0.3985,   0.02859 }, 
 {  -1.500,  -0.3429,   0.02935 }, 
 {  -1.250,  -0.2907,   0.02981 }, 
 {  -1.000,  -0.2043,   0.03039 }, 
 {  -0.750,  -0.1382,   0.03057 }, 
 {  -0.500,  -0.0601,   0.03056 }, 
 {  -0.250,   0.0230,   0.03035 }, 
 {   0.000,   0.0000,   0.03036 }, 
 {   0.250,  -0.0230,   0.03035 }, 
 {   0.500,   0.0600,   0.03056 }, 
 {   0.750,   0.1382,   0.03056 }, 
 {   1.000,   0.2042,   0.03039 }, 
 {   1.250,   0.2906,   0.02981 }, 
 {   1.500,   0.3428,   0.02934 }, 
 {   1.750,   0.3983,   0.02858 }, 
 {   2.000,   0.4321,   0.02816 }, 
 {   2.250,   0.4623,   0.02771 }, 
 {   2.500,   0.4886,   0.02733 }, 
 {   2.750,   0.5068,   0.02726 }, 
 {   3.000,   0.5298,   0.02694 }, 
 {   3.250,   0.5462,   0.02704 }, 
 {   3.500,   0.5651,   0.02701 }, 
 {   3.750,   0.5859,   0.02691 }, 
 {   4.000,   0.6009,   0.02725 }, 
 {   4.250,   0.6208,   0.02729 }, 
 {   4.500,   0.6379,   0.02760 }, 
 {   4.750,   0.6556,   0.02790 }, 
 {   5.000,   0.6764,   0.02807 }, 
 {   5.250,   0.6909,   0.02869 }, 
 {   5.500,   0.7102,   0.02904 }, 
 {   5.750,   0.7272,   0.02962 }, 
 {   6.000,   0.7430,   0.03031 }, 
 {   6.250,   0.7621,   0.03086 }, 
 {   6.500,   0.7795,   0.03163 }, 
 {   6.750,   0.7934,   0.03263 }, 
 {   7.000,   0.8088,   0.03359 }, 
 {   7.250,   0.8247,   0.03460 }, 
 {   7.500,   0.8401,   0.03572 }, 
 {   7.750,   0.8544,   0.03697 }, 
 {   8.000,   0.8680,   0.03838 }, 
 {   8.250,   0.8818,   0.03986 }, 
 {   8.500,   0.8996,   0.04122 }, 
 {   8.750,   0.9077,   0.04310 }, 
 {   9.000,   0.9132,   0.04547 }, 
 {   9.250,   0.9334,   0.04694 }, 
 {   9.500,   0.9249,   0.05030 }, 
 {   9.750,   0.9167,   0.05358 }, 
 {  10.000,   0.9434,   0.05519 }, 
 {  10.250,   0.9177,   0.05971 }, 
 {  10.500,   0.8853,   0.06478 }, 
 {  10.750,   0.8443,   0.07039 }, 
 {  11.000,   0.8024,   0.07755 },
};

std::vector<ValueTupel> NACA_2408 = {
  { -9.250, -0.5158,  0.11581 },
  { -9.000, -0.5047,  0.11139 },
  { -8.750, -0.5244,  0.11068 },
  { -8.500, -0.5127,  0.10634 },
  { -8.250, -0.5012,  0.10208 },
  { -8.000, -0.4921,  0.09805 },
  { -7.750, -0.4902,  0.09472 },
  { -7.250, -0.4846,  0.08830 },
  { -7.000, -0.4846,  0.08558 },
  { -6.750, -0.4857,  0.08261 },
  { -6.500, -0.4754,  0.07928 },
  { -6.250, -0.4705,  0.07597 },
  { -5.500, -0.4465,  0.04776 },
  { -5.250, -0.4272,  0.04311 },
  { -5.000, -0.4072,  0.03925 },
  { -4.750, -0.3850,  0.03567 },
  { -4.500, -0.3609,  0.03247 },
  { -4.250, -0.3387,  0.03039 },
  { -4.000, -0.3137,  0.02816 },
  { -3.750, -0.2883,  0.02626 },
  { -3.500, -0.2627,  0.02455 },
  { -3.250, -0.2371,  0.02316 },
  { -3.000, -0.2125,  0.02182 },
  { -2.750, -0.1858,  0.02018 },
  { -2.500, -0.1645,  0.01790 },
  { -2.250, -0.1218,  0.01600 },
  { -2.000, -0.1006,  0.01601 },
  { -1.750, -0.0793,  0.01607 },
  { -1.500, -0.0579,  0.01616 },
  { -1.250, -0.0363,  0.01628 },
  { -1.000, -0.0147,  0.01644 },
  { -0.750,  0.0068,  0.01664 },
  { -0.500,  0.0283,  0.01687 },
  { -0.250,  0.0497,  0.01714 },
  {  0.000,  0.0709,  0.01747 },
  {  0.250,  0.0918,  0.01784 },
  {  0.500,  0.1125,  0.01826 },
  {  0.750,  0.1327,  0.01875 },
  {  1.000,  0.1525,  0.01931 },
  {  1.250,  0.1719,  0.01995 },
  {  1.500,  0.1907,  0.02066 },
  {  1.750,  0.2090,  0.02146 },
  {  2.000,  0.2269,  0.02235 },
  {  2.250,  0.2794,  0.02355 },
  {  2.500,  0.3384,  0.02464 },
  {  2.750,  0.3896,  0.02553 },
  {  3.000,  0.4445,  0.02628 },
  {  3.250,  0.4931,  0.02685 },
  {  3.500,  0.5394,  0.02726 },
  {  3.750,  0.5930,  0.02730 },
  {  4.000,  0.6410,  0.02703 },
  {  4.250,  0.6805,  0.02667 },
  {  4.500,  0.7211,  0.02559 },
  {  4.750,  0.7519,  0.02392 },
  {  5.000,  0.7757,  0.02239 },
  {  5.250,  0.7942,  0.02177 },
  {  5.500,  0.8098,  0.02164 },
  {  5.750,  0.8138,  0.02235 },
  {  6.000,  0.8187,  0.02581 },
  {  6.250,  0.8401,  0.02806 },
  {  6.500,  0.8678,  0.03023 },
  {  6.750,  0.8978,  0.03268 },
  {  7.000,  0.9250,  0.03533 },
  {  7.250,  0.9498,  0.03812 },
  {  7.500,  0.9712,  0.04130 },
  {  7.750,  0.9897,  0.04448 },
  {  8.000,  1.0078,  0.04832 },
  {  8.250,  1.0203,  0.05265 },
  {  8.500,  1.0281,  0.05723 },
  {  8.750,  1.0307,  0.06193 },
  {  9.000,  1.0303,  0.06688 },
  {  9.250,  1.0245,  0.07189 },
  {  9.500,  0.9834,  0.07910 },
  {  9.750,  0.9758,  0.08468 },
  { 10.000,  0.7879,  0.08467 },
  { 10.250,  0.7659,  0.09338 }
  };

struct Curve {
	std::vector<ValueTupel> data;

	Curve(const std::vector<ValueTupel>& curve_data) : data(curve_data) 
	{
		for (int i = 0; i < data.size() - 1; i++)
			assert(data[i].alpha < data[i+1].alpha);
	}

	static float lerp(float a, float b, float t)
	{
		return a + t * (b - a);
	}
	
	void sample(float alpha, float *cl, float *cd) const
	{
		//assert(data[0].alpha <= alpha && alpha <= data[data.size() - 1].alpha);

		int first = 0;
		int last = data.size() - 1;

		if (alpha < data[first].alpha)
		{
			*cl = data[first].cl;
			*cd = data[first].cd;
			return;
		}

		if (data[last].alpha < alpha)
		{
			*cl = data[last].cl;
			*cd = data[last].cd;
			return;
		}

		for (int i = 0; i < data.size() - 1; i++)
		{
			if (data[i].alpha <= alpha && alpha <= data[i+1].alpha)
			{
				auto t0 = alpha - data[i].alpha;
				auto t1 = data[i+1].alpha - data[i].alpha;
				auto f = t0 / t1;
				*cl = lerp(data[i].cl, data[i+1].cl, f);
				*cd = lerp(data[i].cd, data[i+1].cd, f);
				return;
			}
		}
	}
};

struct Wing {
	const std::string name;
	const float area;
	const glm::vec3 center_of_gravity; // center of gravity relative to rigidbody cg
	const glm::vec3 normal;
	const Curve curve;

	float lift_multiplier = 1.0f;
	float drag_multiplier = 1.0f;

	float lift = 0, drag = 0;

	float log_timer = 1.0f;
	
	Wing(const std::string& wing_name, const glm::vec3& cg_offset, float wing_area, const Curve& aerodynamics, const glm::vec3& wing_normal)
		: name(wing_name), 
		center_of_gravity(cg_offset), 
		area(wing_area),  
		curve(aerodynamics), 
		normal(wing_normal)
	{}

	void apply_forces(phi::RigidBody& rigid_body, float dt, bool log)
	{
		auto local_velocity = (glm::inverse(rigid_body.rotation) * rigid_body.velocity) 
			+ glm::cross(rigid_body.angular_velocity, center_of_gravity);

		//auto local_velocity = glm::inverse(rigid_body.rotation) * rigid_body.velocity;

		auto local_speed = glm::length(local_velocity);

		if (local_speed <= 0.0f)
			return;

		assert(local_speed > 0.0f);

		float speed2 = local_speed * local_speed;

		auto drag_direction = glm::normalize(-local_velocity);
		auto lift_direction = glm::normalize(
			glm::cross(glm::cross(drag_direction, normal), drag_direction)
		);

		float tmp = glm::dot(drag_direction, normal);
		tmp = clamp(tmp, -1, 1);
		float angle_of_attack = glm::degrees(asin(tmp));

		float lift_coefficient, drag_coefficient;
		curve.sample(angle_of_attack, &lift_coefficient, &drag_coefficient);

		lift = lift_coefficient * lift_multiplier * speed2 * area; 
		drag = drag_coefficient * drag_multiplier * speed2 * area; 

		// both in body coordinates
		auto force = lift * lift_direction + drag * drag_direction;
		auto torque = glm::cross(center_of_gravity, force);

		rigid_body.add_relative_force(force);
		rigid_body.add_relative_torque(torque);

		if ((log_timer += dt) > log_intervall && log)
		{
			log_timer = 0;
#if 1
			std::cout << "######### [ " << name << " ] #######" << std::endl;
			std::cout << "aoa = " << angle_of_attack  << std::endl;
			std::cout << "lift = " << lift << std::endl;
			std::cout << "drag = " << drag  << std::endl;
			std::cout << "force = " << force  << std::endl;
			std::cout << "torque = " << torque  << std::endl;
			std::cout << "lift_dir = " << lift_direction << std::endl;
			std::cout << "drag_dir = " << drag_direction << std::endl;
			std::cout << "####################################" << std::endl;
#endif
		}

	}
};

inline float meter_per_second(float kilometer_per_hour)
{
	return kilometer_per_hour / 3.6f;
}

inline float kilometer_per_hour(float meters_per_second)
{
	return meters_per_second * 3.6f;
}

struct Engine {
	float thrust = 10000.0f;

	void apply_forces(phi::RigidBody& rigid_body)
	{
		rigid_body.add_relative_force(glm::vec3(thrust, 0.0f, 0.0f));

		// TODO: implement torque from propeller
	}
};

struct Aircraft {

	phi::RigidBody rigid_body;

	Wing left_wing;
	Wing right_wing;
	Wing wing;
	Wing rudder;
	Wing elevator;

	Engine engine;

	float log_timer = 1.0f;


	// Cessna 172
	Aircraft(const glm::vec3& position, const glm::vec3& velocity)
		: rigid_body(1600.0f), // mass in kg
		wing("wing",			glm::vec3(-0.0f, 1.0f, 0.0f),   16.17f, Curve(NACA_2408), phi::UP),
		left_wing("left_wing",	glm::vec3(-0.0f, 1.0, -2.75f), 16.17f / 2.0f, Curve(NACA_2408), phi::UP),
		right_wing("right_wing",glm::vec3(-0.0f, 1.0, 2.75f),  16.17f / 2.0f, Curve(NACA_2408), phi::UP),
		elevator("elevator",	glm::vec3(-5.0f, 0.0f, 0.0f),    2.0f, Curve(NACA_0015), phi::UP),
		rudder("rudder",		glm::vec3(-5.0f, 0.0f, 0.0f),   1.04f, Curve(NACA_0015), phi::RIGHT)
	{
		rigid_body.position = position;
		rigid_body.velocity = velocity;
	}

	void update(float dt)
	{
#if 1
		left_wing.apply_forces(rigid_body, dt, true);
		right_wing.apply_forces(rigid_body, dt, true);
#else
		wing.apply_forces(rigid_body, dt, true);
#endif
#if 1
		elevator.apply_forces(rigid_body, dt, false);
		rudder.apply_forces(rigid_body, dt, false);
#endif


		engine.apply_forces(rigid_body);

		float lift = rigid_body.get_force().y;
		float gravity = phi::g * rigid_body.mass;
		rigid_body.add_force(phi::DOWN * gravity); 

		if ((log_timer += dt) > log_intervall)
		{
#if 1
			log_timer = 0;
			std::cout << "########## airplane ##############" << std::endl;
			std::cout << "height: " << rigid_body.position.y << std::endl;
			std::cout << "speed: " << kilometer_per_hour(glm::length(rigid_body.velocity)) << std::endl;
			std::cout << "vertical speed: " << kilometer_per_hour(rigid_body.velocity.y) << std::endl;
			std::cout << "torque: " << rigid_body.get_torque() << std::endl;
			std::cout << "force: " << rigid_body.get_force() << std::endl;
			std::cout << "gravity: " << gravity << ", lift = " << lift << std::endl;
			std::cout << "##################################" << std::endl;
#endif
		}

		rigid_body.update(dt);
	}
};

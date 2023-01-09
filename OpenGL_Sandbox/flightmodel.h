#pragma once

#include "phi.h"

#include <cmath>

struct ValueTupel {
	float alpha;
	float cl;
	float cd;
};


/*
  NACA 0015
   alpha    CL        CD    
  ------ -------- --------- 
*/

std::vector<ValueTuple> naca_0015 = {
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




struct Curve {
	std::vector<ValueTupel> data;

	Curve(const std::vector<ValueTupel>& curve_data) : data(curve_data) 
	{
		for (int i = 0; i < data.size() - 1; i++)
		{
			assert(data[i].alpha < data[i+1].alpha);
		}
	}
	
	void sample(float alpha, float *cl, float *cd) const
	{
		for (int i = 0; i < data.size() - 1; i++)
		{
			if (data[i].alpha <= alpha && alpha <= data[i+1].alpha)
			{
				auto t0 = alpha - data[i].alpha;
				auto t1 = data[i+1].alpha - data[i].alpha;
        		auto f = t0 / t1;
				cl* = std::lerp(data[i].cl, data[i+1].cl, f);
				cd* = std::lerp(data[i].cd, data[i+1].cd, f);
        		return;
			}
		}
	}
};

struct Wing {
	const float area;
	const glm::vec3 offset;
	const glm::vec3 normal;
	const Curve curve;

	Wing(const glm::vec3& position_offset, float wing_area)
		: offset(position_offset), area(wing_area), normal(phi::UP)
	{}

	// drag coefficient
	float get_cd_at_aoa(float aoa) const
	{
		return 0.0f;
	}

	// lift coefficient
	float get_cl_at_aoa(float aoa) const
	{
		return 0.0f;
	}

	float get_lift(float aoa, float speed)
	{
		float cl = get_cl_at_aoa(aoa);
		return speed * speed * cl * area;
	}

	float get_drag(float aoa, float speed)
	{
		float cd = get_cd_at_aoa(aoa);
		return speed * speed * cd * area;
	}
	
	void apply_forces(phi::RigidBody& rigid_body)
	{
		auto velocity = rigid_body.get_point_velocity(offset);
		auto lift_direction = glm::normalize(glm::cross(velocity, phi::RIGHT));
		auto drag_direction = glm::normalize(-velocity);

		auto local_velocity = velocity * glm::inverse(rigid_body.rotation); // TODO: account for rotation, adds more velocity
		auto local_speed = glm::length(local_velocity);

		auto angle_of_attack = glm::angle(local_velocity, phi::FORWARD);

		float cl, cd;
		curve.sample(angle_of_attack, &cl, &cd);

		float speed2 = local_speed * local_speed;

		float lift = speed2 * cl * area; 
		float drag = speed2 * cd * area; 

		rigid_body.add_force_at_position(lift_direction * lift, offset);
		rigid_body.add_force_at_position(drag_direction * drag, offset);
	}
};

struct Engine {
	float rpm;
	void apply_forces(phi::RigidBody& rigid_body) {}
};

struct Airplane {

	phi::RigidBody rigid_body;

	Wing wing;
	Wing rudder;
	Wing elevator;

	Airplane(const glm::vec3& position, const glm::vec3& velocity, float mass)
		: 
		rigid_body(position, glm::vec3(0.0f), mass, phi::RigidBody::cube_inertia_tensor(glm::vec3(1.0f), mass)),
		wing(glm::vec3(0.5f, 0.0f, 0.0f), 10.0f),
		elevator(glm::vec3(-1.0f, 0.0f, 0.0f), 2.5f),
		rudder(glm::vec3(-1.0f, 0.1f, 0.0f), 2.0f)
	{
	}

	void update(float dt)
	{
		wing.apply_forces(rigid_body);
		elevator.apply_forces(rigid_body);
		rudder.apply_forces(rigid_body);
		rigid_body.update(dt);
	}
};

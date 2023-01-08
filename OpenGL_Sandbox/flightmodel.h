#pragma once

#include "phi.h"

struct Wing {
	
	float area;
	glm::vec3 offset;

	Wing(const glm::vec3& position_offset, float wing_area)
		: offset(position_offset), area(wing_area)
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
		float lift_coefficient = get_cl_at_aoa(aoa);
		return speed * speed * lift_coefficient * area;
	}

	float get_drag(float aoa, float speed)
	{
		float cd = get_cd_at_aoa(aoa);
		return speed * speed * cd * area;
	}


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
		auto lift_direction = glm::normalize(glm::cross(rigid_body.velocity, phi::RIGHT));
		auto drag_direction = glm::normalize(-rigid_body.velocity);
		auto rudder_lift_direction = glm::normalize(glm::cross(rigid_body.velocity, phi::UP));

		auto local_velocity = rigid_body.velocity * glm::inverse(rigid_body.rotation); // TODO: account for rotation, adds more velocity
		auto local_speed	= glm::length(local_velocity);

		auto angle_of_attack		= glm::angle(local_velocity, phi::FORWARD);
		auto rudder_angle_of_attack = glm::angle(local_velocity, phi::FORWARD); // TODO

		rigid_body.add_force_at_position(lift_direction * wing.get_lift(angle_of_attack, local_speed), wing.offset);
		rigid_body.add_force_at_position(drag_direction * wing.get_drag(angle_of_attack, local_speed), wing.offset);

		rigid_body.add_force_at_position(lift_direction * elevator.get_lift(angle_of_attack, local_speed), elevator.offset);
		rigid_body.add_force_at_position(drag_direction * elevator.get_drag(angle_of_attack, local_speed), elevator.offset);

		rigid_body.add_force_at_position(rudder_lift_direction * rudder.get_lift(rudder_angle_of_attack, local_speed), rudder.offset);
		rigid_body.add_force_at_position(drag_direction * rudder.get_drag(rudder_angle_of_attack, local_speed), rudder.offset);


		
		

	}
};
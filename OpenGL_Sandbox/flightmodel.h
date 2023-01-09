#pragma once

#include "phi.h"

struct Wing {
	const float area;
	const glm::vec3 offset;
	const glm::vec3 normal;

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

		rigid_body.add_force_at_position(lift_direction * get_lift(angle_of_attack, local_speed), offset);
		rigid_body.add_force_at_position(drag_direction * get_drag(angle_of_attack, local_speed), offset);
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

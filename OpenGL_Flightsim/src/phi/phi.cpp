/*********************************************************************************
 * Version: v0.3
 *
 * 'phi.h' is a simple, header-only 3D rigidbody physics library based
 * on 'Physics for Game Developers, 2nd Edition' by David Bourg and Bryan Bywalec.
 *
 * All units are SI, x is forward, y is up and z is right.
 *
 * MIT License
 *
 * Copyright (c) 2023 Jakob Maier
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *********************************************************************************/

#include "phi.h"

namespace phi
{

Logger::Logger(const std::string& path)
{
  file = new std::ofstream(path, std::ios::out);
  assert(file->is_open());
  *file << std::format("time,x,y,z") << std::endl;
}

std::string Logger::to_csv(const glm::vec3& v) { return std::format("{}, {}, {}", v.x, v.y, v.z); }

std::string Logger::to_csv(const glm::quat& q) { return std::format("{}, {}, {}, {}", q.x, q.y, q.z, q.w); }

void Logger::log(const RigidBody* rb, float dt)
{
  timer -= dt;
  time += dt;

  if (timer < 0 && file->is_open()) {
    timer = intervall;

    *file << time << ", " << to_csv(rb->position) << ", " << to_csv(rb->rotation) << ", " << to_csv(rb->velocity)
          << ", " << to_csv(rb->angular_velocity) << ", " << std::endl;
  }
}

}  // namespace phi
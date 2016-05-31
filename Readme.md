Kunlaboro [![Build Status](https://travis-ci.org/ace13/Kunlaboro.svg?branch=rewrite)](https://travis-ci.org/ace13/Kunlaboro) [![Build status](https://ci.appveyor.com/api/projects/status/eqe00q7ej7vrj33m/branch/rewrite?svg=true)](https://ci.appveyor.com/project/ace13/kunlaboro/branch/rewrite)
=========

[Source](https://github.com/ace13/Kunlaboro) | [Issues](https://github.com/ace13/Kunlaboro/issues) | [Documentation](https://ace13.github.io/Kunlaboro)

So I see you've stumbled upon this little project of mine.
Kunlaboro - which is esperanto and means *cooperation* - is a C++ Entity-Component System licensed under the MIT license.

Currently undergoing a rewrite to provide a higher performance data-driven approach to components, while still providing a high-level message passing system.

Requirements
------------

Kunlaboro is built using many C++11 and 14 features, therefore it requires a reasonably modern compiler to work properly.

Current development is done on Visual Studio 2015, with additional testing on GCC-4.9 and Clang-3.4. Other compilers that support C++14 will most likely also work.

TODO
----

- ~~Low level, high performance, event system.~~
  - ~~Events indexed by POD structs containing event data.~~
  - ~~Fast iteration and calling of registered events.~~
- ~~High level, acceptable performance, message passing system.~~
  - ~~Messages indexed by hashed string values (32/64-bit).~~
  - Message passing done in an RPC-like way.
- Functions for shrinking arrays, collecting garbage.
  - Clear unused bitfield slots, reduce size on bit removal.
  - Run garbage collection on memory pools, on user request.
- Better creation of POD components?
  - Look into possibility of having true POD components.
- ~~Improve job queue~~
  - Allow for reusing queue without restarting threads.
- Clean up code, forward declare more things.
  - Include inline through headers.

Code Examples
-------------

3D Particle system;

```c++
struct Position : public Kunlaboro::Component
{
	float X, Y, Z;
};
struct Velocity : public Kunlaboro::Component
{
	float dX, dY, dZ;
};
struct Friction : public Kunlaboro::Component
{
	float Friction;
};
struct Lifetime : public Kunlaboro::Component
{
	float Time;
};

class ParticleSystem
{
public:
	ParticleSystem(Kunlaboro::EntitySystem& es)
		: mES(es)
	{ }

	void update(float dt)
	{
		mDT = dt;

		auto iter = Kunlaboro::EntityView(mES);
		iter.withComponents<Kunlaboro::Match_All, Position, Velocity>.forEach(iterate);
		iter.withComponents<Kunlaboro::Match_All, Velocity, Friction>.forEach(iterate);
		iter.withComponents<Kunlaboro::Match_All, Lifetime>.forEach(iterate);
	}

	void addParticle(float x, float y, float z, float dx, float dy, float dz)
	{
		auto ent = mES.entityCreate();
		ent.addComponent<Position>(x, y, z);
		ent.addComponent<Velocity>(dx, dy, dz);
	}

	void addParticle(float x, float y, float z, float dx, float dy, float dz, float life)
	{
		auto ent = mES.entityCreate();
		ent.addComponent<Position>(x, y, z);
		ent.addComponent<Velocity>(dx, dy, dz);
		ent.addComponent<Lifetime>(life);
	}

private:
	void iterate(const Kunlaboro::Entity& ent, Position& pos, Velocity& vel)
	{
		pos.X += dX * mDT;
		pos.Y += dY * mDT;
		pos.Z += dZ * mDT;
	}
	void iterate(const Kunlaboro::Entity& ent, Velocity& vel, Friction& fric)
	{
		float frictionDelta = 1 - fric.Friction * mDT;

		vel.dX *= frictionDelta;
		vel.dY *= frictionDelta;
		vel.dZ *= frictionDelta;
	}
	void iterate(const Kunlaboro::Entity& ent, Lifetime& time)
	{
		time.Time -= mDT;

		if (time.Time <= 0)
			ent.destroy();
	}

	float mDT;
};
```

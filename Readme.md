[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

# Entity Component System (ECS)
A fully dynamic header only Entity Component System where components can be added or removed to entities whenever.

## What is ECS?

From [Wikipedia](https://en.wikipedia.org/wiki/Entity_component_system):

>`Entity` An entity represents a general-purpose object. In a game engine context, for example, every coarse game object is represented as an entity. Usually, it only consists of a unique id. Implementations typically use a plain integer for this.

>`Component` A component labels an entity as possessing a particular aspect, and holds the data needed to model that aspect. For example, every game object that can take damage might have a Health component associated with its entity. Implementations typically use structs, classes, or associative arrays.

>`System` A system is a process which acts on all entities with the desired components. For example a physics system may query for entities having mass, velocity and position components, and iterate over the results doing physics calculations on the sets of components for each entity.

>The behavior of an entity can be changed at runtime by systems that add, remove or modify components. This eliminates the ambiguity problems of deep and wide inheritance hierarchies often found in Object-Oriented Programming techniques that are difficult to understand, maintain, and extend. Common ECS approaches are highly compatible with, and are often combined with, data-oriented design techniques. Data for all instances of a component are commonly stored together in physical memory, enabling efficient memory access for systems which operate over many entities.

## Features
- Header only library for easy integration.
- Arena memory allocator of the components allocated on the stack.
- Quick iteration over entities due to it being efficiently packed which reduces cache misses.
- Dynamic adding / removing of components.
- Heavy use of templates and concepts to make misuse of library harder and error message clearer.
- Safe concurrent processing of systems.
- Extensive unit testing of library.

## Using this ECS
Create a container, components it should handle are coded in upfront:
```c++
ecs::ECSManager<int, std::string> ecs;
```

AddEntity an entity to the container:
```c++
ecs::EntityID entity = ecs.AddEntity();
```

AddEntity two components to the entity:
```c++
ecs.AddEntity(entity, std::string("Hej"));
ecs.AddEntity(entity, 5);
```

Fill upp and loop over a system with its active entities that has active components:
```c++
ecs::ECSManager<int, float, std::string> ecs;
auto entity1 = ecs.AddEntity();
auto entity2 = ecs.AddEntity();
auto entity3 = ecs.AddEntity();

// Fill up container with components
ecs.AddEntity(entity1, 1);
ecs.AddEntity(entity1, std::string("Hello"));
//ecs.AddEntity(entity1, 5.0f); // No float component on entity1

ecs.AddEntity(entity2, 2);
ecs.AddEntity(entity2, std::string("World"));
ecs.AddEntity(entity2, 5.0f);

ecs.AddEntity(entity3, 3);
//ecs.AddEntity(entity3, "!"); // No string component on entity3
ecs.AddEntity(entity3, 5.0f);

// Will match entity1 and entity2
std::string output;
for (auto [strVal, intVal]: ecs.GetSystem<std::string, int>()) {
    output += strVal + " - " + std::to_string(intVal) + " ";
}
ASSERT_EQ(output, "Hello - 1 World - 2 ");

float fsum = 0;
int isum = 0;
// Will match entity2 and entity3
for (auto [intVal, floatVal]: ecs.GetSystem<int, float>()) {
    isum += intVal;
    fsum += floatVal;
}
ASSERT_EQ(fsum, 10.0f);
ASSERT_EQ(isum, 5);
```

# To install
## Header only method
1. Clone `EntityComponentSystem.h` to your project and include header.

### Dependencies
- C++20

## To run test suite
1. Clone `EntityComponentSystem.h` to your project.
2. Install dependencies.
3. Include the CMakeList in your cmake structure.

### Dependencies
- C++20
- GTest

### To install all dependencies using Conan [optional]
This library uses the PackageManager [Conan](https://conan.io) for its dependencies, and all dependencies can be found in `conantfile.txt`.
1. Install conan `pip3 install conan`
2. Go to the build folder that cmake generates.
3. Run `conan install ..` see [installing dependencies](https://docs.conan.io/en/1.7/using_packages/conanfile_txt.html)
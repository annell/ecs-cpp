# Entity Component System (ECS)
A fully dynamic header only Entity Component System where components can be added or removed to entities whenever.

## What is ECS?

From [Wikipedia](https://en.wikipedia.org/wiki/Entity_component_system):

>`Entity` An entity represents a general-purpose object. In a game engine context, for example, every coarse game object is represented as an entity. Usually, it only consists of a unique id. Implementations typically use a plain integer for this.

>`Component` A component labels an entity as possessing a particular aspect, and holds the data needed to model that aspect. For example, every game object that can take damage might have a Health component associated with its entity. Implementations typically use structs, classes, or associative arrays.

>`System` A system is a process which acts on all entities with the desired components. For example a physics system may query for entities having mass, velocity and position components, and iterate over the results doing physics calculations on the sets of components for each entity.

>The behavior of an entity can be changed at runtime by systems that add, remove or modify components. This eliminates the ambiguity problems of deep and wide inheritance hierarchies often found in Object Oriented Programming techniques that are difficult to understand, maintain, and extend. Common ECS approaches are highly compatible with, and are often combined with, data-oriented design techniques. Data for all instances of a component are commonly stored together in physical memory, enabling efficient memory access for systems which operate over many entities.

## Features
- Header only library for easy integration.
- Arena memory allocator of the components.
- Quick iteration over entities due to it being efficiently packed which reduces cache misses.
- Dynamic adding / removing of components.
- Heavy use of templates and requirements to make misuse of library harder.

## Using this ECS
Create a container, components it should handle are coded in upfront:
```c++
    ecs::ECSManager<int, std::string> ecs;
```

Add a entity to the container:
```c++
    ecs::EntityID entity = ecs.MakeEntity();
```

Add two components to the entity:
```c++
    ecs.Add(entity, std::string("Hej"));
    ecs.Add(entity, 5);
```

Fill upp and loop over active entities that has active components:
```c++
ecs::ECSManager<int, float, std::string> ecs;
auto entity1 = ecs.MakeEntity();
auto entity2 = ecs.MakeEntity();
auto entity3 = ecs.MakeEntity();

// Fill up container with components
ecs.Add(entity1, 1);
ecs.Add(entity1, std::string("Hello"));
//ecs.Add(entity1, 5.0f); // No float component on entity1

ecs.Add(entity2, 2);
ecs.Add(entity2, std::string("World"));
ecs.Add(entity2, 5.0f);

ecs.Add(entity3, 3);
//ecs.Add(entity3, "!"); // No string component on entity3
ecs.Add(entity3, 5.0f);

// Will match entity1 and entity2
std::string output;
for (auto [strVal, intVal]: ecs.FilterEntities<std::string, int>()) {
    output += strVal + " - " + std::to_string(intVal) + " ";
}
ASSERT_EQ(output, "Hello - 1 World - 2 ");

float fsum = 0;
int isum = 0;
// Will match entity2 and entity3
for (auto [intVal, floatVal]: ecs.FilterEntities<int, float>()) {
    isum += intVal;
    fsum += floatVal;
}
ASSERT_EQ(fsum, 10.0f);
ASSERT_EQ(isum, 5);
```

## Dependencies
- C++20
- Conan

## To install all dependencies
This library uses the PackageManager [Conan](https://conan.io) for its dependencies, and all dependencies can be found in `conantfile.txt`.
1. Install conan `pip3 install conan`
2. Go to the build folder that cmake generates.
3. Run `conan install ..` see [installing dependencies](https://docs.conan.io/en/1.7/using_packages/conanfile_txt.html)
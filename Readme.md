# Entity Component System
A fully dynamic header only Entity Component System where components can be added or removed to entities whenever.

Create a container:
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

## Features
- Header only library for easy integration.
- Arena allocator of the memory.
- Quick iteration over entities due to it beeing efficently packed.
- Dynamic adding / removing of components.
- Heavy use of templates and requirements to make missuse of library harder.

## Dependencies
- C++20
- Conan

## To install all dependencies
This library uses the PackageManager [Conan](https://conan.io) for its dependencies, and all dependencies can be found in `conantfile.txt`.
1. Install conan `pip3 install conan`
2. Go to the build folder that cmake generates.
3. Run `conan install ..` see [installing dependencies](https://docs.conan.io/en/1.7/using_packages/conanfile_txt.html)
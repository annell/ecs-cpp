//
// Created by Stefan Annell on 2023-01-01.
//

#include "EntityComponentSystem.h"
#include <gtest/gtest.h>
#include <future>

TEST(ECS, GetLastSlot) {
    ecs::ECSManager<int, std::string> ecs;
    auto entity = ecs.AddEntity();
    ecs.RemoveEntity(entity);
}

TEST(ECS, AddEntity) {
    ecs::ECSManager<int, std::string> ecs;

    auto entity = ecs.AddEntity();
    auto entity2 = ecs.AddEntity();

    ASSERT_EQ(entity.GetId(), 0);
    ASSERT_EQ(entity2.GetId(), 1);
}

TEST(ECS, MakeEntityOverflow) {
    ecs::ECSManager<int> ecs;

    for (int i = 0; i < ecs::ECSManager<int>::NumberOfSlots - 1; i++) {
        auto entity = ecs.AddEntity();
        ASSERT_EQ(entity.GetId(), i);
    }
    EXPECT_THROW(auto entity2 = ecs.AddEntity(), std::out_of_range);
}

TEST(ECS, NotSharedSpace) {
    ecs::ECSManager<int, std::string> ecs;

    ecs::EntityID entity = ecs.AddEntity();
    ecs::EntityID entity2 = ecs.AddEntity();
    ecs.Add(entity, std::string("Hej"));
    ecs.Add(entity2, 5);


    ASSERT_TRUE(ecs.Has<std::string>(entity));
    ASSERT_FALSE(ecs.Has<int>(entity));

    ASSERT_FALSE(ecs.Has<std::string>(entity2));
    ASSERT_TRUE(ecs.Has<int>(entity2));
}

TEST(ECS, SharedSpace) {
    ecs::ECSManager<int, std::string> ecs;

    ecs::EntityID entity = ecs.AddEntity();
    ecs.Add(entity, std::string("Hej"));
    ecs.Add(entity, 5);

    ASSERT_TRUE(ecs.Has<std::string>(entity));
    ASSERT_TRUE(ecs.Has<int>(entity));
}

TEST(ECS, ReadValue) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.AddEntity();

    ASSERT_FALSE(ecs.Has<int>(entity));
    EXPECT_THROW(auto ret = ecs.Get<int>(entity), std::invalid_argument);

    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has<int>(entity));
    ASSERT_EQ(ecs.Get<int>(entity), 5);
}

TEST(ECS, AddTwiceError) {
    ecs::ECSManager<int> ecs;
    ecs::EntityID entity = ecs.AddEntity();

    ecs.Add(entity, 5);
    EXPECT_THROW(ecs.Add(entity, 5), std::logic_error);
}

TEST(ECS, ReadValueSeveralEntities) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.AddEntity();
    ecs::EntityID entity2 = ecs.AddEntity();

    ASSERT_FALSE(ecs.Has<int>(entity));
    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has<int>(entity));
    ASSERT_EQ(ecs.Get<int>(entity), 5);

    ASSERT_FALSE(ecs.Has<int>(entity2));
    ecs.Add(entity2, 42);
    ASSERT_TRUE(ecs.Has<int>(entity2));
    ASSERT_EQ(ecs.Get<int>(entity2), 42);
}

TEST(ECS, ReadValueSeveralEntitiesAndComponents) {
    ecs::ECSManager<int, std::string> ecs;

    ecs::EntityID entity = ecs.AddEntity();
    ecs::EntityID entity2 = ecs.AddEntity();

    ASSERT_FALSE(ecs.Has<std::string>(entity));
    ASSERT_FALSE(ecs.Has<int>(entity));
    ecs.Add(entity, 5);
    ASSERT_FALSE(ecs.Has<std::string>(entity));
    ASSERT_TRUE(ecs.Has<int>(entity));
    ASSERT_EQ(ecs.Get<int>(entity), 5);

    ASSERT_FALSE(ecs.Has<std::string>(entity2));
    ASSERT_FALSE(ecs.Has<int>(entity2));
    ecs.Add(entity2, 42);
    ASSERT_FALSE(ecs.Has<std::string>(entity2));
    ASSERT_TRUE(ecs.Has<int>(entity2));
    ASSERT_EQ(ecs.Get<int>(entity2), 42);

    ASSERT_FALSE(ecs.Has<std::string>(entity));
    ecs.Add(entity, std::string("Hej"));
    ASSERT_TRUE(ecs.Has<std::string>(entity));
    ASSERT_TRUE(ecs.Has<int>(entity));
    ASSERT_EQ(ecs.Get<int>(entity), 5);
    ASSERT_EQ(ecs.Get<std::string>(entity), "Hej");

    ASSERT_FALSE(ecs.Has<std::string>(entity2));
    ecs.Add(entity2, std::string("World"));
    ASSERT_TRUE(ecs.Has<std::string>(entity2));
    ASSERT_TRUE(ecs.Has<int>(entity2));
    ASSERT_EQ(ecs.Get<int>(entity2), 42);
    ASSERT_EQ(ecs.Get<std::string>(entity2), "World");
    ASSERT_EQ(ecs.Get<int>(entity), 5);
    ASSERT_EQ(ecs.Get<std::string>(entity), "Hej");
}

TEST(ECS, InvalidEntityHasComponent) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.AddEntity();
    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has<int>(entity));
    ASSERT_EQ(ecs.Get<int>(entity), 5);

    auto fakeEntity = ecs::EntityID(24);
    EXPECT_THROW(auto output = ecs.Has<int>(fakeEntity), std::out_of_range);
    EXPECT_THROW(auto output = ecs.Get<int>(fakeEntity), std::out_of_range);
}

TEST(ECS, InvalidEntityHasEntity) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.AddEntity();
    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has<int>(entity));
    ASSERT_EQ(ecs.Get<int>(entity), 5);
    ASSERT_TRUE(ecs.HasEntity(entity));

    auto fakeEntity = ecs::EntityID(24);
    ASSERT_FALSE(ecs.HasEntity(fakeEntity));

    auto fakeEntity2 = ecs::EntityID(99999);
    EXPECT_THROW(auto ret = ecs.HasEntity(fakeEntity2), std::out_of_range);
}

TEST(ECS, CheckLastSlot) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.AddEntity();
    ASSERT_TRUE(ecs.HasEntity(entity));
    ASSERT_EQ(entity.GetId(), 0);
    auto fakeEntity = ecs::EntityID(1);
    ASSERT_EQ(fakeEntity.GetId(), 1);
    ASSERT_FALSE(ecs.HasEntity(fakeEntity));

    auto fakeEntity2 = ecs::EntityID(-1);
    ASSERT_EQ(fakeEntity2.GetId(), -1);
    EXPECT_THROW(auto ret = ecs.HasEntity(fakeEntity2), std::out_of_range);
}

TEST(ECS, RemoveEntity) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.AddEntity();
    ASSERT_TRUE(ecs.HasEntity(entity));
    ecs.RemoveEntity(entity);
    ASSERT_FALSE(ecs.HasEntity(entity));
}

TEST(ECS, ReclaimId) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.AddEntity();
    ASSERT_TRUE(ecs.HasEntity(entity));
    ASSERT_EQ(entity.GetId(), 0);
    ecs.RemoveEntity(entity);
    ASSERT_FALSE(ecs.HasEntity(entity));

    ecs::EntityID entity2 = ecs.AddEntity();
    ASSERT_EQ(entity2.GetId(), 0);
    ASSERT_TRUE(ecs.HasEntity(entity2));
}

TEST(ECS, RemoveCleanupComponents) {
    ecs::ECSManager<int> ecs;

    ASSERT_EQ(ecs.Size(), 0);
    ecs::EntityID entity = ecs.AddEntity();
    ASSERT_EQ(ecs.Size(), 1);
    ASSERT_TRUE(ecs.HasEntity(entity));
    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has<int>(entity));
    ecs.RemoveEntity(entity);
    ASSERT_EQ(ecs.Size(), 0);
    ASSERT_FALSE(ecs.HasEntity(entity));

    ASSERT_THROW(ecs.RemoveEntity(entity), std::logic_error);
    ASSERT_EQ(ecs.Size(), 0);

    ecs::EntityID entity2 = ecs.AddEntity();
    ASSERT_EQ(ecs.Size(), 1);
    ASSERT_EQ(entity2.GetId(), 0);
    ASSERT_TRUE(ecs.HasEntity(entity2));
    ASSERT_FALSE(ecs.Has<int>(entity2));
}

TEST(ECS, RemoveComponent) {
    ecs::ECSManager<int> ecs;

    auto entity = ecs.AddEntity();
    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.HasEntity(entity));
    ASSERT_TRUE(ecs.Has<int>(entity));
    ecs.Remove<int>(entity);
    ASSERT_TRUE(ecs.HasEntity(entity));
    ASSERT_FALSE(ecs.Has<int>(entity));

    ASSERT_THROW(ecs.Remove<int>(entity), std::logic_error);
}

TEST(ECS, LoopEntities) {
    ecs::ECSManager<int, std::string> ecs;
    {
        int callCount = 0;
        for (auto &entity: ecs) {
            callCount++;
        }
        ASSERT_EQ(callCount, 0);
    }

    {
        auto entity = ecs.AddEntity();
        ecs.Add(entity, 5);
        int callCount = 0;
        for (auto &e: ecs) {
            callCount++;
            auto [i, str] = e.activeComponents;
            EXPECT_EQ(e.id, entity);
            EXPECT_FALSE(str.active);
            EXPECT_TRUE(i.active);
        }
        ASSERT_EQ(callCount, 1);
    }
    {
        auto entity = ecs.AddEntity();
        ecs.Add(entity, 2);
        ecs.Add(entity, std::string("hej"));

        int callCount = 0;
        for (auto &e: ecs) {
            callCount++;
            if (e.id == entity) {
                auto [str, i] = e.activeComponents;
                EXPECT_TRUE(str.active);
                EXPECT_TRUE(i.active);
                auto result = ecs.Has<int, std::string>(e.id);
                EXPECT_TRUE(result);
            }
        }
        ASSERT_EQ(callCount, 2);
        ecs.RemoveEntity(entity);
        callCount = 0;
        for (auto &e: ecs) {
            callCount++;
        }
        ASSERT_EQ(callCount, 1);
        ASSERT_FALSE(ecs.HasEntity(entity));
    }
}

TEST(ECS, LoopOnceWithFilter) {
    ecs::ECSManager<int, std::string> ecs;

    auto entity = ecs.AddEntity();
    ecs.Add(entity, 5);
    ecs.Add<std::string>(entity, "string");
    int count = 0;
    for (auto [val1, val2]: ecs.GetSystem<int, std::string>()) {
        ASSERT_EQ(val1, 5);
        ASSERT_EQ(val2, "string");
        count++;
    }
    ASSERT_EQ(count, 1);
}

TEST(ECS, LoopMultipleWithFilter) {
    ecs::ECSManager<int, std::string> ecs;
    {
        auto entity = ecs.AddEntity();
        ecs.Add(entity, 5);
        ecs.Add<std::string>(entity, "one");
    }
    {
        auto entity = ecs.AddEntity();
        ecs.Add<std::string>(entity, "two");
    }
    {
        auto entity = ecs.AddEntity();
        ecs.Add(entity, 6);
    }
    {
        auto entity = ecs.AddEntity();
        ecs.Add(entity, 7);
        ecs.Add<std::string>(entity, "three");
    }

    {
        int count = 0;
        for (auto [val1, val2]: ecs.GetSystem<int, std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 2);
    }

    {
        int count = 0;
        for (auto [val]: ecs.GetSystem<int>()) {
            count++;
        }
        ASSERT_EQ(count, 3);
    }

    {
        int count = 0;
        for (auto [val]: ecs.GetSystem<std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 3);
    }
}

TEST(ECS, RemoveEntityAndLoop) {
    using ECSContainer = ecs::ECSManager<int, std::string>;
    ECSContainer ecs;
    auto entity = ecs.AddEntity();
    ecs.Add(entity, 5);
    ecs.Add<std::string>(entity, "one");
    auto entity2 = ecs.AddEntity();
    ecs.Add<std::string>(entity2, "two");
    auto entity3 = ecs.AddEntity();
    ecs.Add(entity3, 6);

    {
        int count = 0;
        for (auto [val]: ecs.GetSystem<std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 2);

        auto system = ecs.GetSystem<std::string>();
        std::for_each(system.begin(), system.end(), [&](const auto &it) {
            auto [val] = it;
            count++;
        });
        ASSERT_EQ(count, 4);
    }

    ecs.Remove<std::string>(entity2);
    {
        int count = 0;
        for (auto [val]: ecs.GetSystem<std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 1);
    }

    ecs.Remove<int>(entity);
    {
        int count = 0;
        for (auto [val]: ecs.GetSystem<int>()) {
            count++;
        }
        ASSERT_EQ(count, 1);
    }

    ecs.Remove<int>(entity3);
    {
        int count = 0;
        for (auto [val]: ecs.GetSystem<int>()) {
            count++;
        }
        ASSERT_EQ(count, 0);
    }

    ecs.Remove<std::string>(entity);
    {
        int count = 0;
        for (auto [val]: ecs.GetSystem<std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 0);
    }
}

TEST(ECS, ReadmeShowcase) {
    ecs::ECSManager<int, float, std::string> ecs;
    auto entity1 = ecs.AddEntity();
    auto entity2 = ecs.AddEntity();
    auto entity3 = ecs.AddEntity();

// Fill up container with components
    ecs.Add(entity1, 1);
    ecs.Add(entity1, std::string("Hello"));
//ecs.AddEntity(entity1, 5.0f); // No float component on entity1

    ecs.Add(entity2, 2);
    ecs.Add(entity2, std::string("World"));
    ecs.Add(entity2, 5.0f);

    ecs.Add(entity3, 3);
//ecs.AddEntity(entity3, "!"); // No string component on entity3
    ecs.Add(entity3, 5.0f);

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
}

TEST(ECS, FindInSystem) {
    ecs::ECSManager<int, float, std::string> ecs;
    auto entity1 = ecs.AddEntity();
    auto entity2 = ecs.AddEntity();
    auto entity3 = ecs.AddEntity();

// Fill up container with components
    ecs.Add(entity1, 1);
    ecs.Add(entity1, std::string("Hello"));
//ecs.AddEntity(entity1, 5.0f); // No float component on entity1

    ecs.Add(entity2, 2);
    ecs.Add(entity2, std::string("World"));
    ecs.Add(entity2, 5.0f);

    ecs.Add(entity3, 3);
    ecs.Add(entity3, std::string("World"));
    ecs.Add(entity3, 5.0f);

    auto system = ecs.GetSystem<int, std::string>();
    auto isCorrectWorld = [](const auto &it) {
        auto [intVal, stringVal] = it;
        return stringVal == "World" && intVal == 3;
    };

    /*auto it = std::find_if(system.begin(), system.end(), isCorrectWorld);
    ASSERT_TRUE(it != system.end());
    auto [intVal, strVal] = *it;
    ASSERT_EQ(intVal, 3);*/
}

TEST(ECS, DefaultConstrutableConcept) {
    struct DefaultConstrutable {
        int val = 0;
        float bla = 0.0f;
    };

    struct NotDefaultConstrutable {
        NotDefaultConstrutable() = delete;

        NotDefaultConstrutable(int val, float bla) : val(val), bla(bla) {}

        int val = 0;
        float bla = 0.0f;
    };

    static_assert(std::default_initializable<DefaultConstrutable>);
    static_assert(not std::default_initializable<NotDefaultConstrutable>);
}

TEST(ECS, FillHoleTest) {
    ecs::ECSManager<int> ecs;
    auto e1 = ecs.AddEntity();
    auto e2 = ecs.AddEntity();
    auto e3 = ecs.AddEntity();

    ASSERT_EQ(e1.GetId(), 0);
    ASSERT_EQ(e2.GetId(), 1);
    ASSERT_EQ(e3.GetId(), 2);

    // Create a hole in the list of entities
    ecs.RemoveEntity(e2);
    ASSERT_EQ(ecs.Size(), 2);
    EXPECT_THROW(ecs.RemoveEntity(e2), std::logic_error);
    ASSERT_EQ(ecs.Size(), 2);

    // The hole should be filled
    auto e4 = ecs.AddEntity();
    ASSERT_EQ(e4.GetId(), 1);
}

TEST(ECS, FillBigHoleTest) {
    ecs::ECSManager<int> ecs;
    auto e1 = ecs.AddEntity();
    auto e2 = ecs.AddEntity();
    auto e3 = ecs.AddEntity();
    auto e4 = ecs.AddEntity();
    auto e5 = ecs.AddEntity();

    ASSERT_EQ(e1.GetId(), 0);
    ASSERT_EQ(e2.GetId(), 1);
    ASSERT_EQ(e3.GetId(), 2);
    ASSERT_EQ(e4.GetId(), 3);
    ASSERT_EQ(e5.GetId(), 4);
    ASSERT_EQ(ecs.Size(), 5);

    // Create a hole in the list of entities
    ecs.RemoveEntity(e2);
    ecs.RemoveEntity(e3);
    ecs.RemoveEntity(e4);

    ASSERT_EQ(ecs.Size(), 2);

    // The hole should be filled
    auto e6 = ecs.AddEntity();
    ASSERT_EQ(e6.GetId(), 1);
    auto e7 = ecs.AddEntity();
    ASSERT_EQ(e7.GetId(), 2);
    auto e8 = ecs.AddEntity();
    ASSERT_EQ(e8.GetId(), 3);

    // Remove and fill first;
    ecs.RemoveEntity(e1);
    auto e9 = ecs.AddEntity();
    ASSERT_EQ(e9.GetId(), 0);

    // Remove and fill last;
    ecs.RemoveEntity(e5);
    auto e10 = ecs.AddEntity();
    ASSERT_EQ(e10.GetId(), 4);

    ASSERT_EQ(ecs.Size(), 5);
}

TEST(ECS, ReuseSlots) {
    ecs::ECSManager<int> ecs;

    // Will go out of bounds and throw if slots aren't reused.
    for (int i = 0; i < 100000; i++) {
        auto entity = ecs.AddEntity();
        ecs.RemoveEntity(entity);
    }
}

TEST(ECS, ConcurencySystem) {
    ecs::ECSManager<int, float> ecs;
    for (int i = 0; i < ecs::ECSManager<int>::NumberOfSlots - 1; i++) {
        auto val = ecs.AddEntity();
    }

    for (auto [i, f]: ecs.GetSystem<int, float>()) {
        i = 42;
        f = 3.14;
    }

    std::vector<std::future<void>> results;

    auto task = [](auto tuple) {
        auto [i, f] = tuple;
        i += 12;
        f = 2;
    };
    for (auto tuple: ecs.GetSystem<int, float>()) {
        results.push_back(std::async(std::launch::async, task, tuple));
    }

    for (const auto &future: results) {
        future.wait();
    }
    for (auto [i, f]: ecs.GetSystem<int, float>()) {
        ASSERT_EQ(i, 54);
        ASSERT_EQ(f, 2);
    }
}

TEST(ECS, LoopComparision) {
    int count1 = 0;
    int count2 = 0;
    int nrRuns = 100;
    for (int i = 0; i < nrRuns; i++) {

        using namespace std::chrono;
        struct obj {
            int i = 5;
            std::string bla = "hello";
            std::optional<int> oi;
            std::vector<int> ints = {1, 2, 3, 4, 5, 6};
        };
        struct ComplexObject {
            int i = 0;
            std::string str1 = "hello";
            float f1 = 4251414.0f;
            std::string str2 = "world";
            float f2 = 4251414.0f;
            obj o1;
            obj o2;
        };

        ecs::ECSManager<ComplexObject> ecs;
        std::array<ComplexObject, ecs::ECSManager<int>::NumberOfSlots> a1;

        for (int j = 0; j < ecs::ECSManager<int>::NumberOfSlots - 1; j++) {
            auto val = ecs.AddEntity();
            ecs.Add<ComplexObject>(val, {});
            a1[j] = {};
        }

        auto start1 = high_resolution_clock::now();
        for (auto &o: a1) {
            o.i = 5;
        }
        auto end1 = high_resolution_clock::now();
        auto duration1 = duration_cast<nanoseconds>(end1 - start1);

        auto start2 = high_resolution_clock::now();
        for (auto [o]: ecs.GetSystem<ComplexObject>()) {
            o.i = 5;
        }
        auto end2 = high_resolution_clock::now();
        auto duration2 = duration_cast<nanoseconds>(end2 - start2);

        // ECS is about ~2-3 slower than a std::array on non-trivial objects.
        // On trivial objects it's about 100 times slower.
        count1 += duration1.count();
        count2 += duration2.count();
    }
    //ASSERT_EQ(count1/nrRuns, count2/nrRuns);
}

TEST(ECS, ConstrictedTypes) {
    struct [[maybe_unused]] fooType {
        fooType() = delete;
        int bla = 5;
    };
    /**
     * Produces compile errors from class requirements
        ecs::ECSManager<> empty;
        ecs::ECSManager<fooType> defaultInitializable;
        ecs::ECSManager<int&> ref;
        ecs::ECSManager<int*> ptr;
        ecs::ECSManager<const int> const;
        ecs::ECSManager<volatile int> volatile;
     */
}

TEST(ECS, HoleTest) {
    ecs::ECSManager<int> ecs;
    auto e1 = ecs.AddEntity();
    auto e2 = ecs.AddEntity();
    auto e3 = ecs.AddEntity();
    auto e4 = ecs.AddEntity();
    auto e5 = ecs.AddEntity();

    ASSERT_EQ(e1.GetId(), 0);
    ASSERT_EQ(e2.GetId(), 1);
    ASSERT_EQ(e3.GetId(), 2);
    ASSERT_EQ(e4.GetId(), 3);
    ASSERT_EQ(e5.GetId(), 4);
    ASSERT_EQ(ecs.Size(), 5);

    // Create a hole in the list of entities
    ecs.RemoveEntity(e2);
    ecs.RemoveEntity(e3);
    ecs.RemoveEntity(e4);

    ASSERT_EQ(ecs.Size(), 2);

    // The hole should be filled
    auto e6 = ecs.AddEntity();
    ASSERT_EQ(e6.GetId(), 1);
    auto e7 = ecs.AddEntity();
    ASSERT_EQ(e7.GetId(), 2);
    auto e8 = ecs.AddEntity();
    ASSERT_EQ(e8.GetId(), 3);

    // Remove and fill first;
    ecs.RemoveEntity(e1);
    auto e9 = ecs.AddEntity();
    ASSERT_EQ(e9.GetId(), 0);

    // Remove and fill last;
    ecs.RemoveEntity(e5);
    auto e10 = ecs.AddEntity();
    ASSERT_EQ(e10.GetId(), 4);

    ASSERT_EQ(ecs.Size(), 5);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

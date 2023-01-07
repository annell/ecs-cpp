//
// Created by Stefan Annell on 2023-01-01.
//

#include "EntityComponentSystem.h"
#include <gtest/gtest.h>

TEST(ECS, MakeEntity) {
    ecs::ECSManager<int, std::string> ecs;

    auto entity = ecs.MakeEntity();
    auto entity2 = ecs.MakeEntity();

    ASSERT_EQ(entity.GetId(), 0);
    ASSERT_EQ(entity2.GetId(), 1);
}

TEST(ECS, MakeEntityOverflow) {
    ecs::ECSManager<int> ecs;

    for (int i = 0; i < 1023; i++) {
        auto entity = ecs.MakeEntity();
        ASSERT_EQ(entity.GetId(), i);
    }
    EXPECT_THROW(auto entity2 = ecs.MakeEntity(), std::out_of_range);
}

TEST(ECS, NotSharedSpace) {
    ecs::ECSManager<int, std::string> ecs;

    ecs::EntityID entity = ecs.MakeEntity();
    ecs::EntityID entity2 = ecs.MakeEntity();
    ecs.Add(entity, std::string("Hej"));
    ecs.Add(entity2, 5);


    ASSERT_TRUE(ecs.Has<std::string>(entity));
    ASSERT_FALSE(ecs.Has<int>(entity));

    ASSERT_FALSE(ecs.Has<std::string>(entity2));
    ASSERT_TRUE(ecs.Has<int>(entity2));
}

TEST(ECS, SharedSpace) {
    ecs::ECSManager<int, std::string> ecs;

    ecs::EntityID entity = ecs.MakeEntity();
    ecs.Add(entity, std::string("Hej"));
    ecs.Add(entity, 5);

    ASSERT_TRUE(ecs.Has<std::string>(entity));
    ASSERT_TRUE(ecs.Has<int>(entity));
}

TEST(ECS, ReadValue) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.MakeEntity();

    ASSERT_FALSE(ecs.Has<int>(entity));
    EXPECT_THROW(auto ret = ecs.Get<int>(entity), std::invalid_argument);

    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has<int>(entity));
    ASSERT_EQ(ecs.Get<int>(entity), 5);
}

TEST(ECS, AddTwiceError) {
    ecs::ECSManager<int> ecs;
    ecs::EntityID entity = ecs.MakeEntity();

    ecs.Add(entity, 5);
    EXPECT_THROW(ecs.Add(entity, 5), std::logic_error);
}

TEST(ECS, ReadValueSeveralEntities) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.MakeEntity();
    ecs::EntityID entity2 = ecs.MakeEntity();

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

    ecs::EntityID entity = ecs.MakeEntity();
    ecs::EntityID entity2 = ecs.MakeEntity();

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

    ecs::EntityID entity = ecs.MakeEntity();
    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has<int>(entity));
    ASSERT_EQ(ecs.Get<int>(entity), 5);

    auto fakeEntity = ecs::EntityID(24);
    EXPECT_THROW(auto output = ecs.Has<int>(fakeEntity), std::out_of_range);
    EXPECT_THROW(auto output = ecs.Get<int>(fakeEntity), std::out_of_range);
}

TEST(ECS, InvalidEntityHasEntity) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.MakeEntity();
    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has<int>(entity));
    ASSERT_EQ(ecs.Get<int>(entity), 5);
    ASSERT_TRUE(ecs.Has(entity));

    auto fakeEntity = ecs::EntityID(24);
    ASSERT_FALSE(ecs.Has(fakeEntity));

    auto fakeEntity2 = ecs::EntityID(99999);
    EXPECT_THROW(auto ret = ecs.Has(fakeEntity2), std::out_of_range);
}

TEST(ECS, CheckLastSlot) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.MakeEntity();
    ASSERT_TRUE(ecs.Has(entity));
    ASSERT_EQ(entity.GetId(), 0);
    auto fakeEntity = ecs::EntityID(1);
    ASSERT_EQ(fakeEntity.GetId(), 1);
    ASSERT_FALSE(ecs.Has(fakeEntity));

    auto fakeEntity2 = ecs::EntityID(-1);
    ASSERT_EQ(fakeEntity2.GetId(), -1);
    EXPECT_THROW(auto ret = ecs.Has(fakeEntity2), std::out_of_range);
}

TEST(ECS, RemoveEntity) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.MakeEntity();
    ASSERT_TRUE(ecs.Has(entity));
    ecs.Remove(entity);
    ASSERT_FALSE(ecs.Has(entity));
}

TEST(ECS, ReclaimId) {
    ecs::ECSManager<int> ecs;

    ecs::EntityID entity = ecs.MakeEntity();
    ASSERT_TRUE(ecs.Has(entity));
    ASSERT_EQ(entity.GetId(), 0);
    ecs.Remove(entity);
    ASSERT_FALSE(ecs.Has(entity));

    ecs::EntityID entity2 = ecs.MakeEntity();
    ASSERT_EQ(entity2.GetId(), 0);
    ASSERT_TRUE(ecs.Has(entity2));
}

TEST(ECS, RemoveCleanupComponents) {
    ecs::ECSManager<int> ecs;

    ASSERT_EQ(ecs.Size(), 0);
    ecs::EntityID entity = ecs.MakeEntity();
    ASSERT_EQ(ecs.Size(), 1);
    ASSERT_TRUE(ecs.Has(entity));
    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has<int>(entity));
    ecs.Remove(entity);
    ASSERT_EQ(ecs.Size(), 0);
    ASSERT_FALSE(ecs.Has(entity));

    ASSERT_THROW(ecs.Remove(entity), std::logic_error);

    ecs::EntityID entity2 = ecs.MakeEntity();
    ASSERT_EQ(ecs.Size(), 1);
    ASSERT_EQ(entity2.GetId(), 0);
    ASSERT_TRUE(ecs.Has(entity2));
    ASSERT_FALSE(ecs.Has<int>(entity2));
}

TEST(ECS, RemoveComponent) {
    ecs::ECSManager<int> ecs;

    auto entity = ecs.MakeEntity();
    ecs.Add(entity, 5);
    ASSERT_TRUE(ecs.Has(entity));
    ASSERT_TRUE(ecs.Has<int>(entity));
    ecs.Remove<int>(entity);
    ASSERT_TRUE(ecs.Has(entity));
    ASSERT_FALSE(ecs.Has<int>(entity));

    ASSERT_THROW(ecs.Remove<int>(entity), std::logic_error);
}

TEST(ECS, GetMultiple) {
    ecs::ECSManager<int, std::string> ecs;
    auto entity = ecs.MakeEntity();
    ASSERT_FALSE(ecs.Has<int>(entity));
    ecs.Add(entity, 1);
    ASSERT_TRUE(ecs.Has<int>(entity));
    auto result = ecs.Has<int, std::string>(entity);
    ASSERT_FALSE(result);
    ecs.Add(entity, std::string("strrr"));
    result = ecs.Has<int, std::string>(entity);
    ASSERT_TRUE(result);
    auto [data1, data2] = ecs.GetSeveral<int, std::string>(entity);
    ASSERT_EQ(data1, 1);
    ASSERT_EQ(data2, "strrr");
}

TEST(ECS, LoopEntities) {
    ecs::ECSManager<int, std::string> ecs;
    {
        int callCount = 0;
        for (auto &entity : ecs) {
            callCount++;
        }
        ASSERT_EQ(callCount, 0);
    }

    {
        auto entity = ecs.MakeEntity();
        ecs.Add(entity, 5);
        int callCount = 0;
        for (auto &e : ecs) {
            callCount++;
            EXPECT_EQ(e.id, entity);
            EXPECT_TRUE(ecs.IsComponentActive<int>(e.componentsActive));
            EXPECT_FALSE(ecs.IsComponentActive<std::string>(e.componentsActive));
        }
        ASSERT_EQ(callCount, 1);
    }
    {
        auto entity = ecs.MakeEntity();
        ecs.Add(entity, 2);
        ecs.Add(entity, std::string("hej"));

        int callCount = 0;
        for (auto &e : ecs) {
            callCount++;
            if (e.id == entity) {
                EXPECT_TRUE(ecs.IsComponentActive<int>(e.componentsActive));
                EXPECT_TRUE(ecs.IsComponentActive<std::string>(e.componentsActive));
                auto result = ecs.Has<int, std::string>(e.id);
                EXPECT_TRUE(result);
            }
        }
        ASSERT_EQ(callCount, 2);
        ecs.Remove(entity);
        callCount = 0;
        for (auto &e : ecs) {
            callCount++;
        }
        ASSERT_EQ(callCount, 1);
        ASSERT_FALSE(ecs.Has(entity));
    }
}

TEST(ECS, LoopOnceWithFilter) {
    ecs::ECSManager<int, std::string> ecs;

    auto entity = ecs.MakeEntity();
    ecs.Add(entity, 5);
    ecs.Add<std::string>(entity, "string");
    int count = 0;
    for (auto [val1, val2] : ecs.FilterEntities<int, std::string>()) {
        ASSERT_EQ(val1, 5);
        ASSERT_EQ(val2, "string");
        count++;
    }
    ASSERT_EQ(count, 1);
}

TEST(ECS, LoopMultipleWithFilter) {
    ecs::ECSManager<int, std::string> ecs;
    {
        auto entity = ecs.MakeEntity();
        ecs.Add(entity, 5);
        ecs.Add<std::string>(entity, "one");
    }
    {
        auto entity = ecs.MakeEntity();
        ecs.Add<std::string>(entity, "two");
    }
    {
        auto entity = ecs.MakeEntity();
        ecs.Add(entity, 6);
    }
    {
        auto entity = ecs.MakeEntity();
        ecs.Add(entity, 7);
        ecs.Add<std::string>(entity, "three");
    }

    {
        int count = 0;
        for (auto [val1, val2] : ecs.FilterEntities<int, std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 2);
    }

    {
        int count = 0;
        for (auto [val] : ecs.FilterEntities<int>()) {
            count++;
        }
        ASSERT_EQ(count, 3);
    }

    {
        int count = 0;
        for (auto [val] : ecs.FilterEntities<std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 3);
    }
}

TEST(ECS, RemoveEntityAndLoop) {
    using ECSContainer = ecs::ECSManager<int, std::string>;
    ECSContainer ecs;
    auto entity = ecs.MakeEntity();
    ecs.Add(entity, 5);
    ecs.Add<std::string>(entity, "one");
    auto entity2 = ecs.MakeEntity();
    ecs.Add<std::string>(entity2, "two");
    auto entity3 = ecs.MakeEntity();
    ecs.Add(entity3, 6);

    {
        int count = 0;
        for (auto [val] : ecs.FilterEntities<std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 2);

        auto filter = ecs.FilterEntities<std::string>();
        std::for_each(filter.begin(), filter.end(), [&] (const auto& it) {
            auto [val] = it;
            count++;
        });
        ASSERT_EQ(count, 4);
    }

    ecs.Remove<std::string>(entity2);
    {
        int count = 0;
        for (auto [val] : ecs.FilterEntities<std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 1);
    }

    ecs.Remove<int>(entity);
    {
        int count = 0;
        for (auto [val] : ecs.FilterEntities<int>()) {
            count++;
        }
        ASSERT_EQ(count, 1);
    }

    ecs.Remove<int>(entity3);
    {
        int count = 0;
        for (auto [val] : ecs.FilterEntities<int>()) {
            count++;
        }
        ASSERT_EQ(count, 0);
    }

    ecs.Remove<std::string>(entity);
    {
        int count = 0;
        for (auto [val] : ecs.FilterEntities<std::string>()) {
            count++;
        }
        ASSERT_EQ(count, 0);
    }
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#ifndef PGE_SANDBOX_BEHAVIOUR_H
#define PGE_SANDBOX_BEHAVIOUR_H

#include <game_entity.h>
#include <game_behaviour.h>
#include <game_transform.h>
#include <game_animation.h>
#include <input_keyboard.h>

class EntityBehaviour : public pge::game_Behaviour {
    pge::game_Entity            m_entity;
    pge::game_TransformManager* m_transformManager;
    pge::game_AnimationManager* m_animManager;

public:
    EntityBehaviour(const pge::game_Entity& entity, pge::game_TransformManager* transformManager, pge::game_AnimationManager* animManager)
        : m_entity(entity)
        , m_transformManager(transformManager)
        , m_animManager(animManager)
    {}

    void
    Update(float delta) override
    {
        pge::math_Vec3 movement;
        if (pge::input_KeyboardDown(pge::input_KeyboardKey::W)) {
            movement.y += 1;
        }
        if (pge::input_KeyboardDown(pge::input_KeyboardKey::A)) {
            movement.x -= 1;
        }
        if (pge::input_KeyboardDown(pge::input_KeyboardKey::S)) {
            movement.y -= 1;
        }
        if (pge::input_KeyboardDown(pge::input_KeyboardKey::D)) {
            movement.x += 1;
        }

        const pge::game_TransformId tid = m_transformManager->GetTransformId(m_entity);

        static bool wasMoving = false;
        if (pge::math_LengthSquared(movement) > 0) {
            movement = pge::math_Normalize(movement);
            if (!wasMoving) {
                m_animManager->Trigger("move_start");
                wasMoving = true;
            }
            m_transformManager->SetLocalForward(tid, pge::math_Normalize(movement), pge::math_Vec3(0, 0, 1));
        } else if (wasMoving) {
            m_animManager->Trigger("move_stop");
            wasMoving = false;
        }

        const float MOVEMENT_SPEED = 0.1f;
        movement *= MOVEMENT_SPEED;
        m_transformManager->Translate(tid, movement);
    }
};

#endif
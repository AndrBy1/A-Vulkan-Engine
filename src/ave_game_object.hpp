//This file takes models created from mve_model.h and applies texture, positioning, rotation, scale and light interaction

#pragma once

#include "ave_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

//Columns of transformation matrix say where i and j basis vectors will land
//transformations can be combined using multiplication A * B = T
//Matrix multiplication is associative but not commutative A * (B * C) = (A * B) * C, A * B != B * A

namespace ave
{
    struct TransformComponent
    {
        glm::vec3 translation{};
        glm::vec3 scale{ 1.f, 1.f, 1.f };
        glm::vec3 rotation{};

        // Matrix corresponds to translate * Ry * Rx * Rz * scale transformation
        // Rotation convention uses tait-bryan angles with axis order Y(1), X(2), Z(3)
        // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
        // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
        // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
        glm::mat4 mat4();
        glm::mat3 normalMatrix();
        
    };

    struct PointLightComponent {
        float lightIntensity = 1.f;
    };

    class AveGameObject {
    public:
		//id_t is the type for the unique identifier of each game object
        using id_t = unsigned int;
        using Map = std::unordered_map<id_t, AveGameObject>;

        static AveGameObject createGameObject() {
            static id_t currentId = 0;
            return AveGameObject{ currentId++ };
        }

        static AveGameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

        AveGameObject() = default;
        AveGameObject(const AveGameObject&) = delete;
        AveGameObject& operator=(const AveGameObject&) = delete;
        AveGameObject(AveGameObject&&) = default;
        AveGameObject& operator=(AveGameObject&&) = default;

        VkDescriptorImageInfo attachTextureFromFile(const std::string& filepath);
        void setTextureDescriptor(VkDescriptorSet descriptor);

        //bool collides = true;
        id_t getId() { return id; }
        VkDescriptorSet getTextureDescriptor() const { return textureDescriptor; }

		//this is set to shared ptr because multiple game objects can share the same model
        std::shared_ptr<AveModel> model{};

        std::unique_ptr<AveImage> textureImage;
        VkDescriptorSet textureDescriptor = VK_NULL_HANDLE;

        glm::vec3 color{};
        TransformComponent transform{};

        //Optional pointer components
        std::unique_ptr<PointLightComponent> pointLight = nullptr;

    private:
        AveGameObject(id_t objId) : id{ objId } {}

        id_t id;
    };
} // namespace ave

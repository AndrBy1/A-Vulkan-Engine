#pragma once

//libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


namespace ave
{
    class AveCamera{
        public:
    
        void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

        void setPerspectiveProjection(float fovy, float aspect, float near, float far);

        void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
        void setViewTarget(glm::vec3 target, glm::vec3 direction, glm::vec3 up = glm::vec3(0.f, -1.f, 0.f)); //if want camera locked on specific point in space
        void setViewYXZ(glm::vec3 position, glm::vec3 rotation); //rotation in radians

        const glm::mat4& getProjection() const {return projectionMatrix;}
        const glm::mat4& getView() const {return viewMatrix;}
        const glm::mat4& getInverseView() const {return inverseViewMatrix;}
        const glm::vec3 getPosition() const {return glm::vec3(inverseViewMatrix[3]);}

        private:
        //the projection matrix defines how 3D points are projected onto a 2D screen. 
        //each element represents a specific transformation or scaling factor used in the projection process.
        //projectionMatrix rows represent the x, y, z, and w coordinates in homogeneous coordinates. Homogeneous coordinates is the extra w to facilitate various transformations, including translation, rotation, and scaling.
        //projectionMatrix columns represent the transformation applied to each of these coordinates.
        glm::mat4 projectionMatrix{1.f};
        
		//the view matrix defines the camera's position and orientation in the 3D world.
        glm::mat4 viewMatrix{1.f};
        
        //the purpose of the inverse view matrix is to transform coordinates from camera space back to world space.
        //this is useful for various calculations, such as determining the camera's position in the world or transforming objects relative to the camera's perspective.
		glm::mat4 inverseViewMatrix{1.f};
    };
    
} // namespace vke

//thanks to ChatGPT and Copilot

#pragma once

#include "ave_game_object.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL //need for gtx
#include <glm/gtx/quaternion.hpp>

#include <iostream>

namespace ave{
    struct Rigidbody{
        glm::vec3 position;
        glm::vec3 velocity{0.f};
        glm::vec3 angularVelocity{0.f};

        glm::quat rotation;
        glm::vec3 force{0.f};
        //int objId;
        float mass;
        float sleepTimer = 0.f;
        bool sleep{false};
        bool collidable{false};
    };

    struct Contact{
        uint32_t a, b; //the two objects in contact
        glm::vec3 normal;
        float penetration; //depth of penetration
    };

	//If you only generate one contact point per collision, you might miss important details about how the objects interact, especially in complex collisions.
	//By having multiple contact points, you can capture a more accurate representation of the collision, leading to better simulation of forces, friction, and overall behavior of the objects involved.
    struct ContactManifold{
        uint32_t A, B;
        glm::vec3 normal;
        std::vector<Contact> points; //contacts between A and B
    };

    struct OBB{
        glm::vec3 center;
        glm::vec3 axis[3]; //local x, y, z axes
        glm::vec3 halfExtents;
    };

	//Separating Axis Theorem: https://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
    struct SATResult{
        bool hit;
        float penetration;
        glm::vec3 normal;
    };

	//AABB (Axis-Aligned Bounding Box) collisions
    struct AABB{
        glm::vec3 min;
        glm::vec3 max;
    };

    struct BoxCollider{
		//half extents is half the size of the box in each dimension
        glm::vec3 halfSize;
        uint32_t objID; //object ID
    };

    struct SphereCollider{
        float radius;
        uint32_t objID; //object ID 
    }; 

    struct Cell { //for a uniform grid
        int x, y, z;
    };

    class PhysicsClass{
        public:
        PhysicsClass();
        ~PhysicsClass();

        void step(float deltaTime);
        void addRigidbody(AveGameObject& obj, float mass = 1.f);
        void addSphereCollider(int objId, float radius);
        void addBoxCollider(int objId, const glm::vec3& halfSize);

        void setSpeed(int objId, const glm::vec3& speed);
        void applyForce(int objId, const glm::vec3& force);

        Cell getCell(const glm::vec3& pos, float cellSize);

        std::unordered_map<int, Rigidbody> rBodies; // int is the game object ID.

        private:
        void buildGrid(float cellSize);

        bool sphereSphere(const Rigidbody& a, const Rigidbody& b, float radiusA, float radiusB, Contact& out);

		// broad phase collision detection using uniform grid
        void detectCollisions();

		//collision response: Impulse solver
        void resolveCollisions();

		//Penetration correction. To prevent sinking due to numerical errors
        void positionalCorrection(Rigidbody& a, Rigidbody& b, const Contact& c);

        // https://research.ncl.ac.uk/game/mastersdegree/gametechnologies/physicstutorials/6accelerationstructures/Physics%20-%20Spatial%20Acceleration%20Structures.pdf
        // broad phase: determine which objects can possibly collide against each other, and then store the potential object collision in a list called collision pair
        // we know objects possibly collide if their bounding volumes (like AABBs or spheres) overlap. This is a quick check that can be done for many objects without much computational cost, allowing us to filter out pairs of objects that are definitely not colliding.
        // Narrow phase collision detection: iterates over the list of potential collision pairs, and determines whether they really are colliding, 
        // and if so, resolves the collision by applying appropriate forces and impulses to the objects involved. 
        void broadPhase();

        void insertCollider(uint32_t colliderIndex, const AABB& aabb, float cellSize);
        AABB computeAABB(const Rigidbody& body, const BoxCollider& boxCollider);

        bool AABBAABB(const AABB& a, const AABB& b, Contact& out);
        bool aabbIntersect(const AABB& a, const AABB& b);

        //OBB vs OBB collision detection using Separating Axis Theorem (SAT)
        OBB buildOBB(uint32_t colliderIndex);
        SATResult testOBBvsOBB(const OBB& a, const OBB& b);

        uint64_t hashCell(const Cell& cell);

        //Integration (semi-implicit Euler): https://math.libretexts.org/Bookshelves/Differential_Equations/Numerically_Solving_Ordinary_Differential_Equations_(Brorson)/01%3A_Chapters/1.07%3A_Symplectic_integrators
        void integrateForces(float dt);
        void integrateVelocity(float dt);
        void applySleep(float dt);

        std::vector<SphereCollider> sphereColliders;
        std::vector<BoxCollider> boxColliders;
        std::vector<Contact> contacts;

        std::unordered_map<uint64_t, std::vector<uint32_t>> grid; //maps cell keys to rigid body indices
        std::vector<std::pair<uint32_t, uint32_t>> aabbPairs; //maps rigid body indices to their AABBs potential collision pairs

    };

}

/*
aabbPairs:
	1: (0,1)
	2: (0,2)
	3: (1,2)
	4: (2,3)
	5: (3,4)
	6: (4,5)


grid:
	CellKey1: [0, 1]
	CellKey2: [0, 2]
	CellKey3: [1, 2]
	CellKey4: [2, 3]
	CellKey5: [3, 4]
	CellKey6: [4, 5]
*/
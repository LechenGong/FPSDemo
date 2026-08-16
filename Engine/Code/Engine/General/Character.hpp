#pragma once

#include <map>
#include <vector>
#include <string>
#include "Engine/Model/ModelUtility.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Quat.hpp"
#include "Engine/General/CharacterState.hpp"
#include "Engine/General/Actor.hpp"
#include "Engine/Math/RaycastUtil.hpp"

class SkeletalMesh;
class SkeletalMeshComponent;
class AnimationSequence;
class AnimationController;
class MeshT;
class Texture;
class Renderer;
class Controller;
class ShapeComponent;
class CharacterMovementComponent;
struct CollisionInfo;

class Character : public Actor
{
	friend class CharacterMovementComponent;
public:
	Character();
	Character( std::string name, SkeletalMesh* skeletalMesh );
	Character( std::string name, std::vector<MeshT*>meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures );
	virtual ~Character();

	//SkeletalMesh* m_skeletalMesh = nullptr;
// 	AnimationSequence* m_currentAnimation = nullptr;
// 	int m_animationIndex = 0;
// 	//std::map<std::string, Texture*> m_textures;
// 	float m_animationClock = 0.f;

	Controller* m_controller = nullptr;
	AnimationController* m_animController = nullptr;

	ShapeComponent* m_boundingCollision = nullptr;
	
	Camera m_camera;
	Vec3 m_cameraArmPivotPos = Vec3::ZERO;
	float m_cameraArmLength = 5.f;

	const float fixedPhysicsDeltaTime = 0.005f;
	float m_physicsTimer = 0.f;

	void SetVelocity( Vec3 const& newVelocity );
	void AddVelocity( Vec3 const& additionVelocity );
	Vec3 GetVelocity() const;

	bool GetIsGrounded() const;

	virtual float GetAboveGroundHeight() = 0;
	virtual RaycastResult3D GetAboveGroundHeight( Vec3 const& pos ) = 0;
	virtual bool DoesBoundingCollisionOverlapWorldAt( Vec3 const& actorWorldPosition ) const;
	virtual Vec3 GetGroundProbePositionAt( Vec3 const& actorWorldPosition ) const;

	bool m_enableGravity = true;

	float m_offGroundTimer = 0.f;
	bool m_inCombat = false;

protected:
	Vec3 m_velocity = Vec3::ZERO;

	void SetIsGrounded( bool flag );
	bool m_isGrounded = false;

	float m_movementSpeed = 1.f;
	float m_sprintParam = 2.f;

	bool m_isUsingIK = false;
	float m_leftFootGroundHeight = 0.f;
	float m_rightFootGroundHeight = 0.f;

public:
	virtual void Update( float deltaSeconds ) override;
	virtual void Render() const override;

	virtual void InitializeAllCollisions();

	virtual std::vector<CollisionInfo>const& GetAllCollisionInfo();
	virtual void InitializeCollisionComponents() {};

	static bool DoCharactersOverlap( Character* charaA, Character* charaB, Vec3& out_mtv_XY_A, Vec3& out_mtv_XY_B );
	static bool DoesAttackHits( Character* attacker, Character* receiver );

	void OnPossessed( Controller* controller );
	void OnUnpossessed();

	void SetMovementSpeed( float newSpeed );
	float GetMovementSpeed() const;

	void SetSprintParam( float newSprintParam );
	float GetSprintParam() const;

	void SetSkeletalMesh( SkeletalMesh* skeletalMesh );
	SkeletalMesh* GetSkeletalMesh() const;
	SkeletalMeshComponent* GetSkeletalMeshComponent() const;

	Mat44 const& GetJointGlobalTransformByIndex( int jointIndex ) const;
	Mat44 const& GetJointGlobalTransformByName( std::string jointName ) const;

 	static bool ToggleCollision( Character* character, int collisionIndex, bool flag );
 	bool SetCollisionEnabled( int collisionIndex, bool flag );

public:
	std::string GetName() { return m_name; }

protected:
	void MoveWithCollision( Vec3 const& worldDisplacement );
	bool TryStartStepFromBlockedMove();
	bool IsStepping() const;
	virtual void CameraArmCollisionCheck();
	virtual void ComponentCollisionCheck();

	SkeletalMeshComponent* m_skeletalMeshComponent;
	CharacterMovementComponent* m_movementComponent = nullptr;
	std::vector<bool> m_collisionsEnabled;

	float m_mass = 100.f;
};

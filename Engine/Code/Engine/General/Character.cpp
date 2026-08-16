#include <cstdarg>

#include "Engine/Core/EngineCommon.hpp"

#include "Engine/General/Character.hpp"

#include "Engine/Renderer/Renderer.hpp"
#include "Engine/General/SkeletalMesh.hpp"
#include "Engine/Animation/AnimationSequence.hpp"
#include "Engine/Animation/AnimationController.hpp"
#include "Engine/Animation/AnimationState.hpp"
#include "Engine/General/MeshT.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/General/Controller.hpp"
#include "Engine/General/CharacterMovementComponent.hpp"
#include "Engine/General/ShapeComponents/CapsuleComponent.hpp"
#include "Engine/General/ShapeComponents/SphereComponent.hpp"
#include "Engine/Core/DebugRenderSystem.hpp"
#include "Engine/Model/ModelUtility.hpp"
#include "Engine/General/SkeletalMeshComponent.hpp"
#include "Engine/Animation/IKSolver.hpp"
#include "Engine/Math/MathUtils.hpp"

Character::Character()
	: Actor()
{
	g_eventSystem->SubscribeEventCallBackFunc( "toggleCollision", &ToggleCollision );
	m_movementComponent = new CharacterMovementComponent( this );
}

Character::Character( std::string name, SkeletalMesh* skeletalMesh )
	: Actor()
{
	m_movementComponent = new CharacterMovementComponent( this );
	if (!GetSkeletalMeshComponent() && !GetSkeletalMesh())
	{
		m_name = name;
		SetSkeletalMesh( skeletalMesh );
		GetSkeletalMeshComponent()->AttachToComponent( m_rootComponent );
	}

	m_boundingCollision = new CapsuleComponent();
	m_boundingCollision->SetCollisionChannel( CollisionChannel::PAWN );
	//m_capsuleComponent->SetScale( Vec3( 4.f, 4.f, 3.4f ) );
	m_boundingCollision->SetLocalPosition( Vec3( 0.f, 0.f, 0.f ) );
}

Character::Character( std::string name, std::vector<MeshT*> meshes, Skeleton const& skeleton, std::map<std::string, Texture*> textures )
	: Actor()
{
	m_movementComponent = new CharacterMovementComponent( this );
	UNUSED( textures );
	m_name = name;
	SetSkeletalMesh( new SkeletalMesh( meshes, skeleton ) );
	GetSkeletalMeshComponent()->AttachToComponent( m_rootComponent );

	m_boundingCollision = new CapsuleComponent();
	//m_capsuleComponent->SetScale( Vec3( 4.f, 4.f, 3.4f ) );
	m_boundingCollision->SetLocalPosition( Vec3( 0.f, 0.f, 0.f ) );
}

Character::~Character()
{
	delete m_movementComponent;
	delete m_skeletalMeshComponent;
}

void Character::SetVelocity( Vec3 const& newVelocity )
{
	m_velocity = newVelocity;
}

void Character::AddVelocity( Vec3 const& additionVelocity )
{
	m_velocity += additionVelocity;
}

Vec3 Character::GetVelocity() const
{
	return m_velocity;
}

void Character::SetIsGrounded( bool flag )
{
	m_isGrounded = flag;
}

bool Character::GetIsGrounded() const
{
	return m_isGrounded;
}

void Character::MoveWithCollision( Vec3 const& worldDisplacement )
{
	if (m_movementComponent)
	{
		m_movementComponent->MoveWithCollision( worldDisplacement );
	}
}

bool Character::TryStartStepFromBlockedMove()
{
	return m_movementComponent && m_movementComponent->TryStartStepFromBlockedMove();
}

bool Character::IsStepping() const
{
	return m_movementComponent && m_movementComponent->IsStepping();
}

bool Character::DoesBoundingCollisionOverlapWorldAt( Vec3 const& actorWorldPosition ) const
{
	UNUSED( actorWorldPosition );
	return false;
}

Vec3 Character::GetGroundProbePositionAt( Vec3 const& actorWorldPosition ) const
{
	return actorWorldPosition;
}

void Character::Update( float deltaSeconds )
{
	if (m_controller)
	{
		m_controller->Update( deltaSeconds );
	}

	if (m_animController)
	{
		m_animController->Update( deltaSeconds );
	}
	if (m_movementComponent)
	{
		m_movementComponent->Update( deltaSeconds );
	}

	if (!m_controller)
	{
		if (GetSkeletalMesh())
		{
			if (m_animController)
			{
				AnimationSequence* currentAnimation = m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentState()->GetAnimation();
				float currentAnimationClock = m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime();

				if (currentAnimation->m_rootTranslation.size() > 0)
				{
					Vec3 rootTranslation = currentAnimation->GetRootTranslationAtTime( currentAnimationClock, deltaSeconds );
					
					Vec3 movementDelta = GetActorLocalTransform().TransformVectorQuantity3D( rootTranslation * GetSkeletalMeshComponent()->GetLocalScale() );
					MoveWithCollision( movementDelta );
				}
				if (currentAnimation->m_rootRotation.size() > 0)
				{
					Quat rootRotation = currentAnimation->GetRootRotationAtTime( currentAnimationClock, deltaSeconds );

					SetActorLocalOrientation( GetActorLocalOrientation() * rootRotation );
				}
				GetSkeletalMesh()->Update();
				AnimationStateMachine* animStateMachine = m_animController->GetStateMachine();
				float currentTimeSeconds = animStateMachine->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime();
				
				float previousTimeSecond = animStateMachine->GetOngoingAnimation( 0 ).GetPreviousAnimationPlaybackTime();
				AnimationSequence* previousAnimaiton = (animStateMachine->GetOngoingAnimation( 0 ).GetPreviousState()) ? animStateMachine->GetOngoingAnimation( 0 ).GetPreviousState()->GetAnimation() : nullptr;
				float crossfadeAlpha = animStateMachine->GetOngoingAnimation( 0 ).GetCrossfadeAlpha();
				GetSkeletalMesh()->UpdateJoints( GetSkeletalMeshComponent()->GetSkeletonGlobalTransform(), currentTimeSeconds, currentAnimation, previousTimeSecond, previousAnimaiton, crossfadeAlpha );
				if (!IsStepping())
				{
					ComponentCollisionCheck();
				}
			}
		}
		return;
	}

	m_physicsTimer += deltaSeconds;

	Vec3 currentInputDirection = m_controller->GetMoveDirection();

	if (currentInputDirection != Vec3::ZERO && !m_controller->IsMovementHindered())
	{
		SetActorLocalOrientation( Quat( Vec3::UP, Atan2Degrees( currentInputDirection.y, currentInputDirection.x ) ) );
	}

	if (!m_animController)
		return;

	AnimationSequence* currentAnimation = m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentState()->GetAnimation();
	float currentAnimationClock = m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentAnimationPlaybackTime();
//  	DebugAddMessage( m_animController->GetStateMachine()->GetOngoingAnimation( 0 ).GetCurrentState()->GetStateName(), deltaSeconds, Rgba8::GREEN, Rgba8::GREEN );
//  	if (m_animController->GetStateMachine()->GetOngoingAnimation( 1 ).GetCurrentState())
//  		DebugAddMessage( m_animController->GetStateMachine()->GetOngoingAnimation( 1 ).GetCurrentState()->GetStateName() + "   " + std::to_string(m_animController->GetStateMachine()->GetOngoingAnimation(1).blendAlpha), deltaSeconds, Rgba8::GREEN, Rgba8::GREEN);
	if (currentAnimation->m_rootRotation.size() > 0)
	{
		Quat rootRotation = currentAnimation->GetRootRotationAtTime( currentAnimationClock, deltaSeconds );

		SetActorLocalOrientation( GetActorLocalOrientation() * rootRotation );
	}
	if (currentAnimation->m_rootTranslation.size() > 0)
	{
		Vec3 rootTranslation = currentAnimation->GetRootTranslationAtTime( currentAnimationClock, deltaSeconds );
		//DebugAddMessage( rootTranslation.ToString(), deltaSeconds, Rgba8::GREEN, Rgba8::GREEN );
		Vec3 movementDelta = GetActorLocalTransform().TransformVectorQuantity3D( rootTranslation * GetSkeletalMeshComponent()->GetLocalScale() );
		//Vec3 position = GetActorLocalPosition() + rootTranslation * GetSkeletalMeshComponent()->GetLocalScale();
		MoveWithCollision( movementDelta );
	}


	if (GetSkeletalMesh())
	{
		GetSkeletalMesh()->Update();
		GetSkeletalMesh()->UpdateJoints( GetSkeletalMeshComponent()->GetSkeletonGlobalTransform(), m_animController->GetStateMachine() );

// 		for (int i = 0; i < GetSkeletalMesh()->m_skeleton.m_joints.size(); i++)
// 		{
// 			Mat44 globalTransform = GetSkeletalMesh()->m_skeleton.m_joints[i].m_globalBindposeInverse.GetInverse();
// 			static const Mat44 XRotation90 = Mat44::CreateXRotationDegrees( 90.f );
// 			static const Mat44 ZRotation90 = Mat44::CreateZRotationDegrees( 90.f );
// 			globalTransform = XRotation90 * globalTransform;
// 			globalTransform = ZRotation90 * globalTransform;
// 			GetSkeletalMeshComponent()->GetSkeletonGlobalTransform()[i] = globalTransform;
// 		}
// 
		if (m_isUsingIK || m_name == "Paladin")
		{
			Skeleton const& skeleton = GetSkeletalMesh()->GetSkeleton();
			std::vector<Mat44>& jointTransforms = GetSkeletalMeshComponent()->GetSkeletonGlobalTransform();
			Mat44 skeletonToWorld = GetSkeletalMeshComponent()->GetWorldTransform();
			Mat44 worldToSkeleton = skeletonToWorld.GetInverse();

			auto isJointValid = [&]( std::string const& jointName, int& outJointIdx )
			{
				outJointIdx = GetSkeletalMesh()->GetJointIndexByName( jointName );
				return outJointIdx >= 0 &&
					outJointIdx < (int)skeleton.m_joints.size() &&
					outJointIdx < (int)jointTransforms.size() &&
					skeleton.m_joints[outJointIdx].m_name == jointName;
			};

			auto applyLegIK = [&]( FootIKConfig const& config, std::string const& toeJointName, float& outGroundHeight )
			{
				int footJointIdx = -1;
				int toeJointIdx = -1;
				if (!isJointValid( config.footJointName, footJointIdx ) || !isJointValid( toeJointName, toeJointIdx ))
				{
					return;
				}

				Vec3 footPos = jointTransforms[footJointIdx].GetTranslation3D();
				Vec3 toePos = jointTransforms[toeJointIdx].GetTranslation3D();
				Vec3 footWorldPos = skeletonToWorld.TransformPosition3D( footPos );
				Vec3 toeWorldPos = skeletonToWorld.TransformPosition3D( toePos );

				RaycastResult3D result = GetAboveGroundHeight( toeWorldPos + Vec3::UP * 2.f );
				if (!result.m_didImpact)
				{
					return;
				}

				outGroundHeight = toeWorldPos.z - result.m_impactPos.z;
				Vec3 targetFootWorldPos = footWorldPos + (result.m_impactPos - toeWorldPos);
				float correctionDistance = (targetFootWorldPos - footWorldPos).GetLength();
				Vec3 groundWorldNormal = result.m_impactNormal.GetNormalized();

				float heightWeight = 1.f - ClampZeroToOne( (outGroundHeight - 0.05f) / (0.35f - 0.05f) );
				float slopeDot = Clamp( DotProduct3D( groundWorldNormal, Vec3::UP ), -1.f, 1.f );
				float slopeWeight = ClampZeroToOne( (slopeDot - CosDegrees( 50.f )) / (CosDegrees( 35.f ) - CosDegrees( 50.f )) );
				float correctionWeight = 1.f - ClampZeroToOne( (correctionDistance - 0.6f) / (1.2f - 0.6f) );
				float ikWeight = heightWeight * slopeWeight * correctionWeight;

				Vec3 targetFootPos = worldToSkeleton.TransformPosition3D( targetFootWorldPos );
				Vec3 groundNormal = worldToSkeleton.TransformVectorQuantity3D( groundWorldNormal ).GetNormalized();
				GetSkeletalMesh()->ApplyFootIK( jointTransforms, config, targetFootPos, groundNormal, ikWeight );
			};

			m_leftFootGroundHeight = 0.f;
			m_rightFootGroundHeight = 0.f;

			FootIKConfig leftLegIK;
			leftLegIK.thighJointName = "mixamorig:LeftUpLeg";
			leftLegIK.kneeJointName = "mixamorig:LeftLeg";
			leftLegIK.footJointName = "mixamorig:LeftFoot";
			applyLegIK( leftLegIK, "mixamorig:LeftToeBase", m_leftFootGroundHeight );

			FootIKConfig rightLegIK;
			rightLegIK.thighJointName = "mixamorig:RightUpLeg";
			rightLegIK.kneeJointName = "mixamorig:RightLeg";
			rightLegIK.footJointName = "mixamorig:RightFoot";
			applyLegIK( rightLegIK, "mixamorig:RightToeBase", m_rightFootGroundHeight );
		}
	}

	if (!IsStepping())
	{
		SetIsGrounded( false );
		ComponentCollisionCheck();
	}

	if (!GetIsGrounded())
	{
		SetIsGrounded( GetAboveGroundHeight() <= 0.1f );
	}

	if (GetIsGrounded())
	{
		m_velocity.z = 0.f;
		m_offGroundTimer = 0.f;
	}
	else
	{
		m_offGroundTimer += deltaSeconds;
	}

	while (m_physicsTimer >= fixedPhysicsDeltaTime)
	{
		if (m_enableGravity)
		{
			m_velocity += Vec3( 0.f, 0.f, -10.f ) * fixedPhysicsDeltaTime;
			//m_velocity.z = Clamp( m_velocity.z, -50.f, 99999.f );
			MoveWithCollision( m_velocity * fixedPhysicsDeltaTime );
		}

		m_physicsTimer -= fixedPhysicsDeltaTime;
	}

	CameraArmCollisionCheck();	
}

void Character::Render() const
{
	g_theRenderer->SetModelConstants( GetSkeletalMeshComponent()->GetWorldTransform() );
	g_theRenderer->SetJointConstants( GetSkeletalMeshComponent()->GetSkeletonGlobalTransform(), GetSkeletalMesh()->GetSkeleton().m_joints );
	GetSkeletalMeshComponent()->Render();
}

void Character::InitializeAllCollisions()
{
	InitializeCollisionComponents();
	auto collisionList = GetAllCollisionInfo();
	m_collisionsEnabled.resize( collisionList.size() );
	for (int i = 0; i < collisionList.size(); i++)
	{
		if (collisionList[i].use == CollisionUsage::BODY ||
			collisionList[i].use == CollisionUsage::PHYSICS ||
			collisionList[i].use == CollisionUsage::INTERACTION)
		{
			m_collisionsEnabled[i] = true;
		}
		else
		{
			m_collisionsEnabled[i] = false;
		}
	}
}

std::vector<CollisionInfo>const& Character::GetAllCollisionInfo()
{
	throw std::logic_error("GetCollisionsInfo() called on base class.");
}

bool Character::DoCharactersOverlap( Character* charaA, Character* charaB, Vec3& out_mtv_XY_A, Vec3& out_mtv_XY_B )
{
	if (!ShapeComponent::DoShapesOverlap( charaA->m_boundingCollision, charaB->m_boundingCollision, out_mtv_XY_A ))
		return false;

	auto collisionInfoA = charaA->GetAllCollisionInfo();
	auto collisionInfoB = charaB->GetAllCollisionInfo();

	Vec3 resultMTV = Vec3::ZERO;
	float maxPenetration = 0.f;
	for (int i = 0; i < collisionInfoA.size(); i++)
	{
		if (!charaA->m_collisionsEnabled[i])
			continue;
		if (collisionInfoA[i].use != CollisionUsage::BODY)
			continue;

		for (int j = 0; j < collisionInfoB.size(); j++)
		{
			if (!charaB->m_collisionsEnabled[j])
				continue;
			if (collisionInfoB[j].use != CollisionUsage::BODY)
				continue;

			Vec3 mtv;
			if (!ShapeComponent::DoShapesOverlap( collisionInfoA[i], charaA, collisionInfoB[j], charaB, mtv ))
				continue;

			resultMTV += mtv;
			maxPenetration = MAX( maxPenetration, mtv.GetLength() );
		}
	}
	if (resultMTV == Vec3::ZERO)
		return false;

	float magitudeA = charaA->m_mass / (charaA->m_mass + charaB->m_mass);
	float magitudeB = charaB->m_mass / (charaA->m_mass + charaB->m_mass);
	resultMTV = resultMTV.GetNormalized();
	Vec3 directionXY = Vec3( resultMTV.x, resultMTV.y, 0.f );
	out_mtv_XY_A = directionXY * magitudeA * maxPenetration;
	out_mtv_XY_B = directionXY * -magitudeB * maxPenetration;
	return true;
}

bool Character::DoesAttackHits( Character* attacker, Character* receiver )
{
	Vec3 placeholder;

	auto attackerCollisions = attacker->GetAllCollisionInfo();
	auto receiverCollisions = receiver->GetAllCollisionInfo();

	for (int i = 0; i < attackerCollisions.size(); i++)
	{
		if (attackerCollisions[i].use != CollisionUsage::ATTACK)
			continue;
		if (!attacker->m_collisionsEnabled[i])
			continue;

		ShapeComponent* attackerCollision = nullptr;
		if (attackerCollisions[i].shape == CollisionShape::CAPSULE)
		{
			attackerCollision = new CapsuleComponent( CapsuleComponent::CreateCapsuleComponent( attackerCollisions[i], attacker, CollisionChannel::PAWN ) );
		}
		else if (attackerCollisions[i].shape == CollisionShape::SPHERE)
		{
			attackerCollision = new SphereComponent( SphereComponent::CreateSphereComponent( attackerCollisions[i], attacker, CollisionChannel::PAWN ) );
		}
		attackerCollision->SetCollisionChannel( PAWN );

		if (!ShapeComponent::DoShapesOverlap( attackerCollision, receiver->m_boundingCollision, placeholder ))
			continue;

		for (int j = 0; j < receiverCollisions.size(); j++)
		{
			if (receiverCollisions[j].use != CollisionUsage::BODY)
				continue;
			if (!receiver->m_collisionsEnabled[j])
				continue;

			ShapeComponent* receiverCollision = nullptr;
			if (receiverCollisions[j].shape == CollisionShape::CAPSULE)
			{
				receiverCollision = new CapsuleComponent( CapsuleComponent::CreateCapsuleComponent( receiverCollisions[j], receiver, CollisionChannel::PAWN ) );
			}
			else if (receiverCollisions[j].shape == CollisionShape::SPHERE)
			{
				receiverCollision = new SphereComponent( SphereComponent::CreateSphereComponent( receiverCollisions[j], receiver, CollisionChannel::PAWN ) );
			}

			if (ShapeComponent::DoShapesOverlap( attackerCollision, receiverCollision, placeholder ))
			{
				delete attackerCollision;
				delete receiverCollision;
				return true;
			}
			delete receiverCollision;
		}
		delete attackerCollision;
	}
	return false;
}

void Character::OnPossessed( Controller* controller )
{
	if (m_controller)
	{
		m_controller->Unpossess();
	}
	m_controller = controller;
	m_controller->m_possessedChara = this;
}

void Character::OnUnpossessed()
{
	if (m_controller)
	{
		m_controller->m_possessedChara = nullptr;
		m_controller = nullptr;
	}
}

void Character::SetMovementSpeed( float newSpeed )
{
	m_movementSpeed = newSpeed;
}

float Character::GetMovementSpeed() const
{
	return m_movementSpeed;
}

void Character::SetSprintParam( float newSprintParam )
{
	m_sprintParam = newSprintParam;
}
float Character::GetSprintParam() const
{
	return m_sprintParam;
}

void Character::SetSkeletalMesh( SkeletalMesh* skeletalMesh )
{
	if (!m_skeletalMeshComponent)
		m_skeletalMeshComponent = new SkeletalMeshComponent;
	m_skeletalMeshComponent->SetSkeletalMesh( skeletalMesh );
}

SkeletalMesh* Character::GetSkeletalMesh() const
{
	if (!m_skeletalMeshComponent)
		return nullptr;
	return m_skeletalMeshComponent->GetSkeletalMesh();
}

SkeletalMeshComponent* Character::GetSkeletalMeshComponent() const
{
	return m_skeletalMeshComponent;
}

Mat44 const& Character::GetJointGlobalTransformByIndex( int jointIndex ) const
{
	return GetSkeletalMeshComponent()->GetSkeletonGlobalTransform()[jointIndex];
}

Mat44 const& Character::GetJointGlobalTransformByName( std::string jointName ) const
{
	return GetSkeletalMeshComponent()->GetSkeletonGlobalTransform()[GetSkeletalMesh()->GetJointIndexByName( jointName )];
}

bool Character::ToggleCollision( Character* character, int collisionIndex, bool flag )
{
	character->SetCollisionEnabled( collisionIndex, flag );
	return false;
}

bool Character::SetCollisionEnabled( int collisionIndex, bool flag )
{
	if (collisionIndex >= m_collisionsEnabled.size() ||
		collisionIndex < 0)
		return false;

	m_collisionsEnabled[collisionIndex] = flag;
	return true;
}

void Character::CameraArmCollisionCheck()
{
	m_camera.m_position = GetActorWorldPosition() + m_cameraArmPivotPos;
	Vec3 cameraArmVectorNormal = m_camera.m_orientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D().GetNormalized();
	Vec3 cameraArmVectorNegateNormal = cameraArmVectorNormal * -1.f;

	m_camera.m_position += cameraArmVectorNegateNormal * m_cameraArmLength;
}

void Character::ComponentCollisionCheck()
{
}

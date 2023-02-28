#include "stdafx.h"
#include "ModelAnimator.h"

ModelAnimator::ModelAnimator(MeshFilter* pMeshFilter):
m_pMeshFilter(pMeshFilter),
m_Transforms(std::vector<DirectX::XMFLOAT4X4>()),
m_IsPlaying(false), 
m_Reversed(false),
m_ClipSet(false),
m_TickCount(0),
m_AnimationSpeed(1.0f)
{
	SetAnimation(0);
}

void ModelAnimator::SetAnimation(UINT clipNumber, bool looping)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(clipNumber);
	//TODO: complete
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Check if clipNumber is smaller than the actual m_AnimationClips vector size
	if (clipNumber < GetClipCount())
	{
		//	Retrieve the AnimationClip from the m_AnimationClips vector based on the given clipNumber
		//	Call SetAnimation(AnimationClip clip)
		SetAnimation(m_pMeshFilter->m_AnimationClips[clipNumber], looping);
	}
	else
	{
		//	Call Reset
		Reset();
		//	Log a warning with an appropriate message
		Logger::LogWarning(L"invalid clipnumber: " + std::to_wstring(clipNumber) + L": There are only " +std::to_wstring(GetClipCount())+ L" Clips\n");
		return;
	}
		
}



void ModelAnimator::SetAnimation(std::wstring clipName, bool looping)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(clipName);
	//TODO: complete
	//Set m_ClipSet to false
	m_ClipSet = false;
	//Iterate the m_AnimationClips vector and search for an AnimationClip with the given name (clipName)
	std::vector<AnimationClip>& clips = m_pMeshFilter->m_AnimationClips;
	std::vector<AnimationClip>::iterator itClip = std::find_if(clips.begin(), clips.end()
		, [&clipName](AnimationClip& clip)->bool {return clip.Name == clipName; });
	//If found,
	if (itClip != clips.end())
	{
	//	Call SetAnimation(Animation Clip) with the found clip
		SetAnimation(*itClip, looping);
	}
	else
	{
		//	Call Reset
		Reset();
		//	Log a warning with an appropriate message
		Logger::LogWarning(L"ClipName: " + clipName + L" Does not exist\n");

	}

}

void ModelAnimator::SetAnimation(AnimationClip clip, bool looping)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(clip);
	//TODO: complete
	//Set m_ClipSet to true
	m_ClipSet = true;
	//Set m_CurrentClip
	m_CurrentClip = clip;
	//Call Reset(false)
	Reset(false);
	m_Looping = looping;
}

void ModelAnimator::Reset(bool pause)
{
	UNREFERENCED_PARAMETER(pause);
	//TODO: complete
	//If pause is true, set m_IsPlaying to false
	if (pause) m_IsPlaying = false;
	//Set m_TickCount to zero
	m_TickCount = 0;
	//Set m_AnimationSpeed to 1.0f
	m_AnimationSpeed = 1.0f;
	//If m_ClipSet is true
	if (m_ClipSet)
	{
		//	Retrieve the BoneTransform from the first Key from the current clip (m_CurrentClip)
		//	Refill the m_Transforms vector with the new BoneTransforms (have a look at vector::assign)
		m_Transforms.assign(m_CurrentClip.Keys[0].BoneTransforms.begin(), m_CurrentClip.Keys[0].BoneTransforms.end());
	}
	else
	{
		//	Create an IdentityMatrix 
		DirectX::XMFLOAT4X4 identity{};
		identity._11 = 1.0f;
		identity._22 = 1.0f;
		identity._33 = 1.0f;
		identity._44 = 1.0f;
		//	Refill the m_Transforms vector with this IdenityMatrix (Amount = BoneCount) (have a look at vector::assign)
		m_Transforms.assign(m_pMeshFilter->m_BoneCount, identity);
	}
}

float ModelAnimator::GetClipProgression() const
{
	return (m_TickCount / m_CurrentClip.Duration);
}


void ModelAnimator::SetAnimationAt(float at)
{
	m_TickCount = at * m_CurrentClip.Duration;
}


void ModelAnimator::Update(const GameContext& gameContext)
{
	PIX_PROFILE();

	UNREFERENCED_PARAMETER(gameContext);
	//TODO: complete
	//We only update the transforms if the animation is running and the clip is set
	if (m_IsPlaying && m_ClipSet)
	{
		//1. 
		//Calculate the passedTicks (see the lab document)
		float passedTicks = gameContext.pGameTime->GetElapsed() * m_CurrentClip.TicksPerSecond * m_AnimationSpeed;
		//Make sure that the passedTicks stay between the m_CurrentClip.Duration bounds (fmod)
		passedTicks = fmod(passedTicks, m_CurrentClip.Duration);
		//2. 
		//IF m_Reversed is true
		if (m_Reversed)
		{
			//	Subtract passedTicks from m_TickCount
			m_TickCount -= passedTicks;
			//	If m_TickCount is smaller than zero, add m_CurrentClip.Duration to m_TickCount
			if (m_TickCount < 0)
			{
				if (m_Looping)
					m_TickCount += m_CurrentClip.Duration;
				else
				{
					Pause();
					return;
				}
			}
		}
		//ELSE
		else
		{
			//	Add passedTicks to m_TickCount
			m_TickCount += passedTicks;
			//	if m_TickCount is bigger than the clip duration, subtract the duration from m_TickCount
			if (m_TickCount > m_CurrentClip.Duration)
			{
				if (m_Looping)
					m_TickCount -= m_CurrentClip.Duration;
				else
				{
					Pause();
					return;
				}
			}
		}
		//3.
		//Find the enclosing keys
		AnimationKey keyA{}, keyB{};
		keyA.Tick = 0;
		keyB.Tick = m_CurrentClip.Duration;
		//Iterate all the keys of the clip and find the following keys:
		//keyA > Closest Key with Tick before/smaller than m_TickCount
		//keyB > Closest Key with Tick after/bigger than m_TickCount
		for (AnimationKey& key : m_CurrentClip.Keys)
		{
			if (key.Tick <= m_TickCount && key.Tick >= keyA.Tick) keyA = key;
			else if (key.Tick >= m_TickCount && key.Tick <= keyB.Tick) keyB = key;
		}


		//4.
		//Interpolate between keys
		//Figure out the BlendFactor (See lab document)
		float blendFactor = (m_TickCount - keyA.Tick) / (keyB.Tick - keyA.Tick); //remap to [0,1]
		//Clear the m_Transforms vector
		m_Transforms.clear();
		//FOR every boneTransform in a key (So for every bone)
		for (size_t i = 0; i < m_pMeshFilter->m_BoneCount; i++)
		{
			//	Retrieve the transform from keyA (transformA)
			DirectX::XMFLOAT4X4 transformA = keyA.BoneTransforms[i];
			// 	Retrieve the transform from keyB (transformB)
			DirectX::XMFLOAT4X4 transformB = keyB.BoneTransforms[i];

			//	Decompose both transforms

			DirectX::XMVECTOR posA, scaleA, rotA;
			DirectX::XMMatrixDecompose(&scaleA, &rotA, &posA, DirectX::XMLoadFloat4x4(&transformA));

			DirectX::XMVECTOR posB, scaleB, rotB;
			DirectX::XMMatrixDecompose(&scaleB, &rotB, &posB, DirectX::XMLoadFloat4x4(&transformB));


			//	Lerp between all the transformations (Position, Scale, Rotation)
			DirectX::XMVECTOR translation = DirectX::XMVectorLerp(posA, posB, blendFactor);
			DirectX::XMVECTOR scale = DirectX::XMVectorLerp(scaleA, scaleB, blendFactor);
			DirectX::XMVECTOR rot = DirectX::XMQuaternionSlerp(rotA, rotB, blendFactor);

			//	Compose a transformation matrix with the lerp-results
			
			DirectX::XMFLOAT4X4 t;
			DirectX::XMStoreFloat4x4(&t,DirectX::XMMatrixTransformation(DirectX::XMVectorZero(), DirectX::XMQuaternionIdentity(), scale, DirectX::XMVectorZero(), rot, translation));
		
			//	Add the resulting matrix to the m_Transforms vector
			m_Transforms.push_back(t);
		}


	}
}

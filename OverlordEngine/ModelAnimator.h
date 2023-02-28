#pragma once
#include "MeshFilter.h"

class ModelAnimator final
{
public:
	ModelAnimator(MeshFilter* pMeshFilter);
	~ModelAnimator() = default;

	ModelAnimator(const ModelAnimator& other) = delete;
	ModelAnimator(ModelAnimator&& other) noexcept = delete;
	ModelAnimator& operator=(const ModelAnimator& other) = delete;
	ModelAnimator& operator=(ModelAnimator&& other) noexcept = delete;
	
	void SetAnimation(std::wstring clipName, bool looping = true);
	void SetAnimation(UINT clipNumber, bool looping = true);
	void SetAnimation(AnimationClip clip, bool looping = true);
	void Update(const GameContext& gameContext);
	void Reset(bool pause = true);
	void Play() { m_IsPlaying = true; }
	void Pause() { m_IsPlaying = false; }
	void SetPlayReversed(bool reverse) { m_Reversed = reverse; }
	void SetAnimationSpeed(float speedPercentage) { m_AnimationSpeed = speedPercentage; }
	void SetAnimationAt(float at);

	void SetLooping(bool looping) { m_Looping = looping; }
	bool IsPlaying() const { return m_IsPlaying; }
	bool IsReversed() const { return m_Reversed; }
	float GetAnimationSpeed() const { return m_AnimationSpeed; }
	UINT GetClipCount() const { return (UINT)m_pMeshFilter->m_AnimationClips.size(); }
	std::wstring GetClipName() const { return m_ClipSet?m_CurrentClip.Name:L""; }
	float GetClipProgression() const;
	std::vector<DirectX::XMFLOAT4X4> GetBoneTransforms() const { return m_Transforms; }

private:
	AnimationClip m_CurrentClip;
	MeshFilter* m_pMeshFilter;
	std::vector<DirectX::XMFLOAT4X4> m_Transforms;
	bool m_IsPlaying, m_Reversed, m_ClipSet;
	float m_TickCount, m_AnimationSpeed;
	bool m_Looping;
};


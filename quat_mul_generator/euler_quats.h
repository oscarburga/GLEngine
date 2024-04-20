namespace glm
{
	 quat fromYaw(float Yaw);
	 quat fromPitch(float Pitch);
	 quat fromYawPitch(float Yaw, float Pitch);
	 quat fromPitchYaw(float Pitch, float Yaw);
	 quat fromRoll(float Roll);
	 quat fromYawRoll(float Yaw, float Roll);
	 quat fromRollYaw(float Roll, float Yaw);
	 quat fromPitchRoll(float Pitch, float Roll);
	 quat fromRollPitch(float Roll, float Pitch);
	 quat fromYawPitchRoll(float Yaw, float Pitch, float Roll);
	 inline quat fromYawPitchRoll(const vec3& yawPitchRoll) { return fromYawPitchRoll(yawPitchRoll.x, yawPitchRoll.y, yawPitchRoll.z); }
	 quat fromYawRollPitch(float Yaw, float Roll, float Pitch);
	 inline quat fromYawRollPitch(const vec3& yawRollPitch) { return fromYawRollPitch(yawRollPitch.x, yawRollPitch.y, yawRollPitch.z); }
	 quat fromPitchYawRoll(float Pitch, float Yaw, float Roll);
	 inline quat fromPitchYawRoll(const vec3& pitchYawRoll) { return fromPitchYawRoll(pitchYawRoll.x, pitchYawRoll.y, pitchYawRoll.z); }
	 quat fromPitchRollYaw(float Pitch, float Roll, float Yaw);
	 inline quat fromPitchRollYaw(const vec3& pitchRollYaw) { return fromPitchRollYaw(pitchRollYaw.x, pitchRollYaw.y, pitchRollYaw.z); }
	 quat fromRollYawPitch(float Roll, float Yaw, float Pitch);
	 inline quat fromRollYawPitch(const vec3& rollYawPitch) { return fromRollYawPitch(rollYawPitch.x, rollYawPitch.y, rollYawPitch.z); }
	 quat fromRollPitchYaw(float Roll, float Pitch, float Yaw);
	 inline quat fromRollPitchYaw(const vec3& rollPitchYaw) { return fromRollPitchYaw(rollPitchYaw.x, rollPitchYaw.y, rollPitchYaw.z); }
}

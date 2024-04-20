quat glm::fromYaw(float Yaw)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosYaw,
		0.0f,
		sinYaw,
		0.0f
	);
}

quat glm::fromPitch(float Pitch)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosPitch,
		sinPitch,
		0.0f,
		0.0f
	);
}

quat glm::fromYawPitch(float Yaw, float Pitch)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosYaw * cosPitch,
		cosYaw * sinPitch,
		sinYaw * cosPitch,
		sinYaw * sinPitch
	);
}

quat glm::fromPitchYaw(float Pitch, float Yaw)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosPitch * cosYaw,
		sinPitch * cosYaw,
		cosPitch * sinYaw,
		 - sinPitch * sinYaw
	);
}

quat glm::fromRoll(float Roll)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosRoll,
		0.0f,
		0.0f,
		sinRoll
	);
}

quat glm::fromYawRoll(float Yaw, float Roll)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosYaw * cosRoll,
		 - sinYaw * sinRoll,
		sinYaw * cosRoll,
		cosYaw * sinRoll
	);
}

quat glm::fromRollYaw(float Roll, float Yaw)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosRoll * cosYaw,
		sinRoll * sinYaw,
		cosRoll * sinYaw,
		sinRoll * cosYaw
	);
}

quat glm::fromPitchRoll(float Pitch, float Roll)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosPitch * cosRoll,
		sinPitch * cosRoll,
		sinPitch * sinRoll,
		cosPitch * sinRoll
	);
}

quat glm::fromRollPitch(float Roll, float Pitch)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosRoll * cosPitch,
		cosRoll * sinPitch,
		 - sinRoll * sinPitch,
		sinRoll * cosPitch
	);
}

quat glm::fromYawPitchRoll(float Yaw, float Pitch, float Roll)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosYaw * cosPitch * cosRoll - sinYaw * sinPitch * sinRoll,
		cosYaw * sinPitch * cosRoll - sinYaw * cosPitch * sinRoll,
		cosYaw * sinPitch * sinRoll + sinYaw * cosPitch * cosRoll,
		cosYaw * cosPitch * sinRoll + sinYaw * sinPitch * cosRoll
	);
}

quat glm::fromYawRollPitch(float Yaw, float Roll, float Pitch)
{
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosYaw * cosRoll * cosPitch - sinYaw *  - sinRoll * sinPitch,
		cosYaw * cosRoll * sinPitch - sinYaw * sinRoll * cosPitch,
		cosYaw *  - sinRoll * sinPitch + sinYaw * cosRoll * cosPitch,
		cosYaw * sinRoll * cosPitch + sinYaw * cosRoll * sinPitch
	);
}

quat glm::fromPitchYawRoll(float Pitch, float Yaw, float Roll)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	return quat(
		cosPitch * cosYaw * cosRoll - sinPitch *  - sinYaw * sinRoll,
		cosPitch *  - sinYaw * sinRoll + sinPitch * cosYaw * cosRoll,
		cosPitch * sinYaw * cosRoll + sinPitch * cosYaw * sinRoll,
		cosPitch * cosYaw * sinRoll - sinPitch * sinYaw * cosRoll
	);
}

quat glm::fromPitchRollYaw(float Pitch, float Roll, float Yaw)
{
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosPitch * cosRoll * cosYaw - sinPitch * sinRoll * sinYaw,
		cosPitch * sinRoll * sinYaw + sinPitch * cosRoll * cosYaw,
		cosPitch * cosRoll * sinYaw + sinPitch * sinRoll * cosYaw,
		cosPitch * sinRoll * cosYaw - sinPitch * cosRoll * sinYaw
	);
}

quat glm::fromRollYawPitch(float Roll, float Yaw, float Pitch)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	return quat(
		cosRoll * cosYaw * cosPitch - sinRoll * sinYaw * sinPitch,
		cosRoll * cosYaw * sinPitch + sinRoll * sinYaw * cosPitch,
		cosRoll * sinYaw * cosPitch - sinRoll * cosYaw * sinPitch,
		cosRoll * sinYaw * sinPitch + sinRoll * cosYaw * cosPitch
	);
}

quat glm::fromRollPitchYaw(float Roll, float Pitch, float Yaw)
{
	const float cosRoll = glm::cos(Roll * 0.5f);
	const float sinRoll = glm::sin(Roll * 0.5f);
	const float cosPitch = glm::cos(Pitch * 0.5f);
	const float sinPitch = glm::sin(Pitch * 0.5f);
	const float cosYaw = glm::cos(Yaw * 0.5f);
	const float sinYaw = glm::sin(Yaw * 0.5f);
	return quat(
		cosRoll * cosPitch * cosYaw - sinRoll *  - sinPitch * sinYaw,
		cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
		cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
		cosRoll *  - sinPitch * sinYaw + sinRoll * cosPitch * cosYaw
	);
}


/*
 * Copyright 2022 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/OrientedBox3.h>

#include <DeepSea/Math/Color.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Random.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Particle/Particle.h>

#include <gtest/gtest.h>

static const unsigned int cIterations = 1000;

TEST(ParticleTest, RandomPositionBox)
{
	dsAlignedBox3f box = {{{-1.0f, -2.0f, -3.0f}}, {{4.0f, 5.0f, 6.0f}}};
	dsParticleVolume volume;
	volume.type = dsParticleVolumeType_Box;
	volume.box = box;

	dsMatrix44f transform;
	dsMatrix44f_makeRotate(&transform, 0.1f, -0.2f, 0.3f);
	transform.columns[3].x = -1.2f;
	transform.columns[3].y = 3.4f;
	transform.columns[3].z = -5.6f;

	dsVector3f epsilonVec = {{1e-5f, 1e-5f, 1e-5f}};
	dsVector3_sub(box.min, box.min, epsilonVec);
	dsVector3_add(box.max, box.max, epsilonVec);
	dsOrientedBox3f referenceBox;
	dsOrientedBox3_fromAlignedBox(referenceBox, box);
	ASSERT_TRUE(dsOrientedBox3f_transform(&referenceBox, &transform));

	dsAlignedBox3f pointBox;
	dsAlignedBox3f_makeInvalid(&pointBox);

	dsRandom random;
	dsRandom_seed(&random, 0);
	dsParticle particle;
	for (unsigned int i = 0; i < cIterations; ++i)
	{
		dsParticle_randomPosition(&particle, &random, &volume, &transform);
		EXPECT_TRUE(dsOrientedBox3f_containsPoint(&referenceBox, &particle.position));
		dsAlignedBox3_addPoint(pointBox, particle.position);
	}

	// Rotated so fuzzy size check.
	dsVector3f size;
	dsAlignedBox3_extents(size, pointBox);
	EXPECT_LT(5.0f, size.x);
	EXPECT_LT(7.0f, size.y);
	EXPECT_LT(9.0f, size.z);
	EXPECT_GT(8.0f, size.x);
	EXPECT_GT(10.0f, size.y);
	EXPECT_GT(12.0f, size.z);
}

TEST(ParticleTest, RandomPositionSphere)
{
	dsParticleVolume volume;
	volume.type = dsParticleVolumeType_Sphere;
	volume.sphere.center.x = 1.2f;
	volume.sphere.center.y = -3.4f;
	volume.sphere.center.z = 5.6f;
	volume.sphere.radius = 7.8f;

	dsMatrix44f transform;
	dsMatrix44f_makeTranslate(&transform, 0.1f, -0.2f, 0.3f);

	dsVector3f transformedCenter;
	dsVector3_add(transformedCenter, volume.sphere.center, transform.columns[3]);

	dsAlignedBox3f pointBox;
	dsAlignedBox3f_makeInvalid(&pointBox);

	dsRandom random;
	dsRandom_seed(&random, 0);
	dsParticle particle;
	for (unsigned int i = 0; i < cIterations; ++i)
	{
		dsParticle_randomPosition(&particle, &random, &volume, &transform);
		float distance = dsVector3f_dist(&particle.position, &transformedCenter);
		EXPECT_GT(volume.sphere.radius + 1e-5f, distance);
		dsAlignedBox3_addPoint(pointBox, particle.position);
	}

	float maxSize = 2*volume.sphere.radius + 1e-5f;
	float minSize = 0.9f*maxSize;
	dsVector3f size;
	dsAlignedBox3_extents(size, pointBox);
	EXPECT_LT(minSize, size.x);
	EXPECT_LT(minSize, size.y);
	EXPECT_LT(minSize, size.z);
	EXPECT_GT(maxSize, size.x);
	EXPECT_GT(maxSize, size.y);
	EXPECT_GT(maxSize, size.z);
}

TEST(ParticleTest, RandomPositionCylinder)
{
	dsParticleVolume volume;
	volume.type = dsParticleVolumeType_Cylinder;
	volume.cylinder.center.x = 1.2f;
	volume.cylinder.center.y = -3.4f;
	volume.cylinder.center.z = 5.6f;
	volume.cylinder.radius = 7.8f;
	volume.cylinder.height = 9.0f;

	dsMatrix44f transform;
	dsMatrix44f_makeTranslate(&transform, 0.1f, -0.2f, 0.3f);

	dsVector3f transformedCenter;
	dsVector3_add(transformedCenter, volume.sphere.center, transform.columns[3]);

	dsAlignedBox3f pointBox;
	dsAlignedBox3f_makeInvalid(&pointBox);

	dsRandom random;
	dsRandom_seed(&random, 0);
	dsParticle particle;
	for (unsigned int i = 0; i < cIterations; ++i)
	{
		dsParticle_randomPosition(&particle, &random, &volume, &transform);
		float distance = dsVector2f_dist((const dsVector2f*)&particle.position,
			(const dsVector2f*)&transformedCenter);
		EXPECT_GT(volume.cylinder.radius + 1e-5f, distance);
		EXPECT_LT(transformedCenter.z - volume.cylinder.height/2 - 1e-5f, particle.position.z);
		EXPECT_GT(transformedCenter.z + volume.cylinder.height/2 + 1e-5f, particle.position.z);
		dsAlignedBox3_addPoint(pointBox, particle.position);
	}

	dsVector3f maxSize = {{volume.cylinder.radius*2 + 1e-5f, volume.cylinder.radius*2 + 1e-5f,
		volume.cylinder.height + 1e-5f}};
	dsVector3f minSize;
	dsVector3_scale(minSize, maxSize, 0.9f);
	dsVector3f size;
	dsAlignedBox3_extents(size, pointBox);
	EXPECT_LT(minSize.x, size.x);
	EXPECT_LT(minSize.y, size.y);
	EXPECT_LT(minSize.z, size.z);
	EXPECT_GT(maxSize.x, size.x);
	EXPECT_GT(maxSize.y, size.y);
	EXPECT_GT(maxSize.z, size.z);
}

TEST(ParticleTest, RandomSize)
{
	dsVector2f sizeRange = {{1.0f, 2.0f}};

	dsAlignedBox2f sizeBox;
	dsAlignedBox2f_makeInvalid(&sizeBox);

	dsRandom random;
	dsRandom_seed(&random, 0);
	dsParticle particle;
	unsigned int rectangleCount = 0;
	for (unsigned int i = 0; i < cIterations; ++i)
	{
		dsParticle_randomSize(&particle, &random, &sizeRange, &sizeRange);
		if (particle.size.x != particle.size.y)
			++rectangleCount;
		dsAlignedBox2_addPoint(sizeBox, particle.size);
	}

	EXPECT_LT(0U, rectangleCount);

	EXPECT_LE(sizeRange.x, sizeBox.min.x);
	EXPECT_LE(sizeRange.x, sizeBox.min.y);
	EXPECT_GE(sizeRange.y, sizeBox.max.x);
	EXPECT_GE(sizeRange.y, sizeBox.max.y);

	dsAlignedBox2f_makeInvalid(&sizeBox);
	rectangleCount = 0;
	for (unsigned int i = 0; i < cIterations; ++i)
	{
		dsParticle_randomSize(&particle, &random, &sizeRange, nullptr);
		if (particle.size.x != particle.size.y)
			++rectangleCount;
		dsAlignedBox2_addPoint(sizeBox, particle.size);
	}

	EXPECT_EQ(0U, rectangleCount);

	EXPECT_LE(sizeRange.x, sizeBox.min.x);
	EXPECT_LE(sizeRange.x, sizeBox.min.y);
	EXPECT_GE(sizeRange.y, sizeBox.max.x);
	EXPECT_GE(sizeRange.y, sizeBox.max.y);
}

TEST(ParticleTest, RandomDirection)
{
	dsVector3f baseDirection = {{-0.3f, 1.2f, -4.5f}};
	dsVector3f_normalize(&baseDirection, &baseDirection);

	dsMatrix33f directionMatrix;
	dsParticle_createDirectionMatrix(&directionMatrix, &baseDirection);

	float angle = 1.2f;
	float cosAngle = cosf(angle);

	dsRandom random;
	dsRandom_seed(&random, 0);
	for (unsigned int i = 0; i < cIterations; ++i)
	{
		dsVector3f direction;
		dsParticle_randomDirection(&direction, &random, &directionMatrix, angle);
		EXPECT_LT(cosAngle - 1e-5f, dsVector3_dot(baseDirection, direction));
		EXPECT_FLOAT_EQ(1.0f, dsVector3f_len(&direction));
	}
}

TEST(ParticleTest, RandomColor)
{
	dsVector2f hueRange = {{12.3f, 45.6f}};
	dsVector2f saturationRange = {{0.3f, 0.7f}};
	dsVector2f valueRange = {{0.2f, 0.5f}};
	dsVector2f alphaRange = {{0.3f, 0.6f}};

	dsAlignedBox3f hsvRange = {{{hueRange.x, saturationRange.x, valueRange.x}},
		{{hueRange.y, saturationRange.y, valueRange.y}}};

	// Hue can shift quite a bit with the conversion to a 32-bit color.
	dsVector3f epsilonVec = {{2.0f, 1e-2f, 1e-2f}};
	dsVector3_sub(hsvRange.min, hsvRange.min, epsilonVec);
	dsVector3_add(hsvRange.max, hsvRange.max, epsilonVec);

	dsAlignedBox3f hsvBox;
	dsAlignedBox3f_makeInvalid(&hsvBox);

	dsRandom random;
	dsRandom_seed(&random, 0);
	dsParticle particle;
	for (unsigned int i = 0; i < cIterations; ++i)
	{
		dsParticle_randomColor(&particle, &random, &hueRange, &saturationRange, &valueRange,
			&alphaRange);
		dsHSVColor hsvColor;
		dsHSVColor_fromColor(&hsvColor, particle.color);
		EXPECT_TRUE(dsAlignedBox3_containsPoint(hsvRange, *(dsVector3f*)&hsvColor)) << hsvColor.h <<
			", " << hsvColor.s << ", " << hsvColor.v;
		dsAlignedBox3_addPoint(hsvBox, *(dsVector3f*)&hsvColor);
		EXPECT_LE(alphaRange.x, hsvColor.a);
		EXPECT_GE(alphaRange.y, hsvColor.a);
	}

	dsVector3f maxSize;
	dsAlignedBox3_extents(maxSize, hsvRange);

	dsVector3f minSize;
	dsVector3_scale(minSize, maxSize, 0.8f);

	dsVector3f hsvSize;
	dsAlignedBox3_extents(hsvSize, hsvBox);

	EXPECT_LT(minSize.x, hsvSize.x);
	EXPECT_LT(minSize.y, hsvSize.y);
	EXPECT_LT(minSize.z, hsvSize.z);
	EXPECT_GT(maxSize.x, hsvSize.x);
	EXPECT_GT(maxSize.y, hsvSize.y);
	EXPECT_GT(maxSize.z, hsvSize.z);
}

TEST(ParticleTest, RandomColorWrapped)
{
	dsVector2f hueRange = {{123.4f, 56.7f}};
	dsVector2f saturationRange = {{0.3f, 0.7f}};
	dsVector2f valueRange = {{0.2f, 0.5f}};
	dsVector2f alphaRange = {{0.0f, 1.0f}};

	dsAlignedBox3f lowerHsvRange = {{{0.0f, saturationRange.x, valueRange.x}},
		{{hueRange.y, saturationRange.y, valueRange.y}}};
	dsAlignedBox3f upperHsvRange = {{{hueRange.x, saturationRange.x, valueRange.x}},
		{{360.0f, saturationRange.y, valueRange.y}}};

	// Hue can shift quite a bit with the conversion to a 32-bit color.
	dsVector3f epsilonVec = {{2.0f, 1e-2f, 1e-2f}};
	dsVector3_sub(lowerHsvRange.min, lowerHsvRange.min, epsilonVec);
	dsVector3_add(lowerHsvRange.max, lowerHsvRange.max, epsilonVec);
	dsVector3_sub(upperHsvRange.min, upperHsvRange.min, epsilonVec);
	dsVector3_add(upperHsvRange.max, upperHsvRange.max, epsilonVec);

	dsAlignedBox3f lowerHsvBox, upperHsvBox;
	dsAlignedBox3f_makeInvalid(&lowerHsvBox);
	dsAlignedBox3f_makeInvalid(&upperHsvBox);

	dsRandom random;
	dsRandom_seed(&random, 0);
	dsParticle particle;
	for (unsigned int i = 0; i < cIterations; ++i)
	{
		dsParticle_randomColor(&particle, &random, &hueRange, &saturationRange, &valueRange,
			&alphaRange);
		dsHSVColor hsvColor;
		dsHSVColor_fromColor(&hsvColor, particle.color);
		if (hsvColor.h <= lowerHsvRange.max.x)
		{
			EXPECT_TRUE(dsAlignedBox3_containsPoint(lowerHsvRange, *(dsVector3f*)&hsvColor)) <<
				hsvColor.h << ", " << hsvColor.s << ", " << hsvColor.v;
			dsAlignedBox3_addPoint(lowerHsvBox, *(dsVector3f*)&hsvColor);
		}
		else
		{
			EXPECT_TRUE(dsAlignedBox3_containsPoint(upperHsvRange, *(dsVector3f*)&hsvColor)) <<
				hsvColor.h << ", " << hsvColor.s << ", " << hsvColor.v;
			dsAlignedBox3_addPoint(upperHsvBox, *(dsVector3f*)&hsvColor);
		}
		EXPECT_LE(alphaRange.x, hsvColor.a);
		EXPECT_GE(alphaRange.y, hsvColor.a);
	}

	dsVector3f maxSize;
	dsAlignedBox3_extents(maxSize, lowerHsvRange);

	dsVector3f minSize;
	dsVector3_scale(minSize, maxSize, 0.8f);

	dsVector3f hsvSize;
	dsAlignedBox3_extents(hsvSize, lowerHsvBox);

	EXPECT_LT(minSize.x, hsvSize.x);
	EXPECT_LT(minSize.y, hsvSize.y);
	EXPECT_LT(minSize.z, hsvSize.z);
	EXPECT_GT(maxSize.x, hsvSize.x);
	EXPECT_GT(maxSize.y, hsvSize.y);
	EXPECT_GT(maxSize.z, hsvSize.z);

	dsAlignedBox3_extents(maxSize, upperHsvRange);
	dsVector3_scale(minSize, maxSize, 0.8f);
	dsAlignedBox3_extents(hsvSize, upperHsvBox);

	EXPECT_LT(minSize.x, hsvSize.x);
	EXPECT_LT(minSize.y, hsvSize.y);
	EXPECT_LT(minSize.z, hsvSize.z);
	EXPECT_GT(maxSize.x, hsvSize.x);
	EXPECT_GT(maxSize.y, hsvSize.y);
	EXPECT_GT(maxSize.z, hsvSize.z);
}

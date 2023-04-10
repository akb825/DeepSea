# Copyright 2023 Aaron Barany
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import math

def eulerToQuaternion(x, y, z):
	halfXRad = math.radians(x)*0.5
	halfYRad = math.radians(y)*0.5
	halfZRad = math.radians(z)*0.5
	cosX = math.cos(halfXRad)
	sinX = math.sin(halfXRad)
	cosY = math.cos(halfYRad)
	sinY = math.sin(halfYRad)
	cosZ = math.cos(halfZRad)
	sinZ = math.sin(halfZRad)

	return (
		cosX*cosY*cosZ + sinX*sinY*sinZ,
		sinX*cosY*cosZ - cosX*sinY*sinZ,
		cosX*sinY*cosZ + sinX*cosY*sinZ,
		cosX*cosY*sinZ - sinX*sinY*cosZ)

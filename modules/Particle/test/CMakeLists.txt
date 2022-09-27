if (NOT GTEST_FOUND OR NOT DEEPSEA_BUILD_TESTS)
	return()
endif()

file(GLOB_RECURSE sources *.cpp *.h)
ds_add_unittest(deepsea_particle_test ${sources})

target_link_libraries(deepsea_particle_test PRIVATE DeepSea::Particle)

ds_set_folder(deepsea_particle_test tests/unit)
add_test(NAME DeepSeaParticleTest COMMAND deepsea_particle_test)

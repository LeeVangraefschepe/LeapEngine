# Game cmake
if(DEFINED IsConsole)
if (IsConsole)
	else()
	if(WIN32)
		set(WIN32_EXECUTABLE WIN32)
	endif()
endif()
endif()

add_executable(UnnamedAdventureGame ${WIN32_EXECUTABLE}
	"main.cpp"
	"Scenes/MainScene.cpp"
	"Components/Transformator.cpp"
	"Components/InfoUI.cpp"
	"Components/FreeCamMovement.cpp"
	"Components/WindowManager.cpp"
	"Components/AudioTester.cpp"
	"Components/SineWaveTerrain.cpp"
    "Components/PrintCollision.cpp"
	"Components/ApplyForces.cpp" 
	"Components/ColliderScaler.cpp" 
	"Components/PrintVelocity.cpp" 
	"Components/ImGuiSceneChooser.cpp"
	"Components/PointConversionTest.cpp" 
	"Scenes/ExtendedCameraTest.cpp"
	"Scenes/ParticleTest.cpp"
	"Scenes/Physics/MeshesScene.cpp")

target_include_directories(
	LeapEngine PUBLIC 
	${PhysicsEngineIncludeDir} 
	${GraphicsEngineIncludeDir}
	${InputEngineIncludeDir}
	${AudioEngineIncludeDir}
	${NetworkingEngineIncludeDir}
	${GLFW_INCLUDE_DIR}
	${GLMIncludeDir} 
	${EngineUtilsIncludeDir})
target_link_libraries(
	LeapEngine PUBLIC 
	PhysicsEngine
	GraphicsEngine
	InputEngine
	AudioEngine
	NetworkingEngine
	EngineUtils)
target_include_directories(UnnamedAdventureGame PUBLIC ${LeapEngineIncludeDir})
target_link_libraries(UnnamedAdventureGame PUBLIC LeapEngine ${GLFW_LIBRARY_DIR})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	target_include_directories(UnnamedAdventureGame PUBLIC ${VLD_INCLUDE_DIR})
	target_link_libraries(UnnamedAdventureGame PUBLIC ${VLD_LIBRARY_DIR})
endif()

# Copies used for .exe
add_custom_command(TARGET UnnamedAdventureGame PRE_LINK
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FMOD_DLL_DIR} $<TARGET_FILE_DIR:UnnamedAdventureGame>
    COMMENT "Copying FMOD DLL..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PHYSX_DLL_DIR} $<TARGET_FILE_DIR:UnnamedAdventureGame>
    COMMENT "Copying PHYSX DLL..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PHYSX_COMMON_DLL_DIR} $<TARGET_FILE_DIR:UnnamedAdventureGame>
    COMMENT "Copying PHYSX COMMON DLL..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PHYSX_COOKING_DLL_DIR} $<TARGET_FILE_DIR:UnnamedAdventureGame>
    COMMENT "Copying PHYSX COOKING DLL..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PHYSX_FOUNDATION_DLL_DIR} $<TARGET_FILE_DIR:UnnamedAdventureGame>
    COMMENT "Copying PHYSX FOUNDATION DLL..."
    COMMAND ${CMAKE_COMMAND} -E copy_directory  ${DATA_FILES} ${DESTINATION_COPY}
    COMMENT "Copying DATA..."
)
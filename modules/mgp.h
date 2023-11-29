
#include "mgp_core.h"

// APP
#include "app/Platform.h"
//#include "app/Gamepad.h"
//#include "app/ScreenDisplayer.h"
#include "app/FirstPersonCamera.h"
#include "app/SceneView.h"

#ifndef __EMSCRIPTEN__
// Audio
#include "audio/AudioController.h"
#include "scene/AudioListener.h"
#include "audio/AudioBuffer.h"
#include "audio/AudioSource.h"

// Physics
#include "physics/PhysicsController.h"
#include "physics/PhysicsConstraint.h"
#include "physics/PhysicsCollisionObject.h"
#include "physics/PhysicsCollisionShape.h"
#include "physics/PhysicsRigidBody.h"
#include "physics/PhysicsGhostObject.h"
#include "physics/PhysicsCharacter.h"
#include "physics/PhysicsVehicle.h"
#include "physics/PhysicsVehicleWheel.h"

// AI
#include "ai/AIController.h"
#include "ai/AIAgent.h"
#include "ai/AIState.h"
#include "ai/AIStateMachine.h"
#endif

// UI
#include "ui/Theme.h"
#include "ui/Control.h"
#include "ui/ControlFactory.h"
#include "ui/Container.h"
#include "ui/Form.h"
#include "ui/Label.h"
#include "ui/Button.h"
#include "ui/CheckBox.h"
#include "ui/TextBox.h"
#include "ui/RadioButton.h"
#include "ui/Slider.h"
#include "ui/ImageControl.h"
#include "ui/JoystickControl.h"
#include "ui/Layout.h"
#include "ui/AbsoluteLayout.h"
#include "ui/VerticalLayout.h"
#include "ui/FlowLayout.h"
#include "ui/ComboBox.h"
#include "ui/TreeView.h"
#include "ui/ScrollContainer.h"
#include "ui/MenuList.h"

// Render
#include "loader/GltfLoader.h"
#include "render/RenderStage.h"
#include "render/FrameBuffer.h"
#include "openGL/GLFrameBuffer.h"
#include "render/RenderPath.h"
#include "openGL/DepthStencilTarget.h"




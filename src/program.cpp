#include "program.hpp"

namespace vke
{
  Program::Program() :
      m_eventRelayer{},
      m_window{m_eventRelayer},
      m_device{m_window},
      m_ecs{},
      m_modelManager{m_device, m_ecs},
      m_renderer{m_device, m_window, m_eventRelayer}
  {
    loadEntities();

    RenderSystemContext renderSystemContext{
      .eventRelayer = m_eventRelayer,
      .ecs = m_ecs,
      .modelManager = m_modelManager,
    };

    m_renderSystem = std::make_unique<RenderSystem>(m_device, renderSystemContext, m_renderer.renderPass(), m_renderer.swapchainExtent());
  }

  Program::~Program()
  {
    for(auto& entity : m_entities)
    {
      m_ecs.destroyEntity(entity);
    }

    // m_model.reset();

    /*
    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr); // duplicate code
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);
    */
  }

  void Program::run()
  {
    using TimeStep = std::chrono::duration<double, std::chrono::seconds::period>;
    using scTimePoint = std::chrono::steady_clock::time_point;

    Camera camera{};
    KeyboardInput cameraController{m_ecs, m_window};
    EntityID cameraEntity{m_ecs.createEntity()};
    m_ecs.addComponent(cameraEntity, cmp::Transform3D{
      .translation{-1.2f, -1.5f, 0.f},
      .rotation{-glm::quarter_pi<double>(), glm::quarter_pi<double>(), 0.f}
    });

    // camera.setViewDirection(glm::vec3{0.f}, glm::vec3{0.0f, 0.0f, 2.5f});
    // camera.setViewTarget(glm::vec3{1.f, -2.f, 3.f}, glm::vec3{0.0f, 0.0f, 2.5f});

    auto const& now{&std::chrono::steady_clock::now};
    scTimePoint frameStartTime{now()};
    scTimePoint frameEndTime{};
    TimeStep timeStep{};

    while(!m_window.shouldClose())
    {
      frameEndTime = now();
      timeStep = frameEndTime - frameStartTime;
      frameStartTime = frameEndTime;

      m_window.poolEvents();
      dispatchEvents();

      auto& translation{m_ecs.getComponent<cmp::Transform3D>(cameraEntity).translation};
      auto& rotation{m_ecs.getComponent<cmp::Transform3D>(cameraEntity).rotation};
      auto aspectRatio{m_renderer.swapchainAspectRatio()};

      ////////////////
      auto& gameObjTransform{m_ecs.getComponent<cmp::Transform3D>(gameObj)};
      double speed{0.5};
      double rotX{gameObjTransform.rotation.x + (speed * timeStep.count())};
      double rotY{gameObjTransform.rotation.y + (speed / 2 * timeStep.count())};
      double rotZ{gameObjTransform.rotation.z + (speed / 3 * timeStep.count())};
      gameObjTransform.rotation.x = glm::mod(rotX, glm::two_pi<double>());
      gameObjTransform.rotation.y = glm::mod(rotY, glm::two_pi<double>());
      gameObjTransform.rotation.z = glm::mod(rotZ, glm::two_pi<double>());
      ////////////////

      cameraController.moveInPlaneXZ(timeStep, cameraEntity);
      // camera.setViewDirection(translation, glm::vec3{0.5f, 0.0f, 1.f});

      camera.setViewRotationZYX(translation, rotation);
      // camera.setViewRotationXYZ(translation, rotation);

      // camera.setOrthographicProjection(aspectRatio, 0.1f, 5.f);
      camera.setPerspectiveProjection(glm::radians(50.f), aspectRatio, 0.1, 10.f);

      // you could draw only when necessary, and repeatedly present the current image.
      // this can avoid needless draw() calls in more static scenes.
      if(m_renderer.beginFrame())
      {
        m_renderer.beginRenderPass();

        m_renderSystem->renderEntities(m_renderer.currentCommandBuffer(), camera, m_entities);

        m_renderer.endRenderPass();
        m_renderer.endFrame();

        m_renderer.present();
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(6));
    }

    vkDeviceWaitIdle(m_device);
  }

  void Program::dispatchEvents()
  {
    // m_eventRelayer.dispatch<event::WindowResized>();
    m_eventRelayer.dispatch<event::InvalidPipeline>();
  }

  void Program::loadEntities()
  {
    m_ecs.registerComponent<cmp::Transform3D>();
    m_ecs.registerComponent<cmp::Common>();
    m_ecs.registerComponent<cmp::Color>();

    cmp::Transform3D transform3D{
      .translation{-2.f, 0.f, 1.f},
      .scale{1.f, 1.f, 1.f},
      .rotation{
        {}, //glm::radians(-15.f),
        {}, //glm::radians(35.f),
        {},
      },
    };

    cmp::Common common{};

    //  EntityID cube{m_ecs.createEntity()};
    gameObj = m_ecs.createEntity();
    auto cube = m_ecs.createEntity();
    auto smoothVase = m_ecs.createEntity();
    auto smallVase = m_ecs.createEntity();

    m_entities.push_back(gameObj);
    m_entities.push_back(cube);
    m_entities.push_back(smoothVase);
    m_entities.push_back(smallVase);

    for(auto& e : m_entities)
    {
      transform3D.translation.x += 1.0;

      m_ecs.addComponent<cmp::Transform3D>(e, transform3D);
      m_ecs.addComponent<cmp::Common>(e, common);
      m_ecs.addComponent<cmp::Color>(e, {});
    }

    m_ecs.getComponent<cmp::Transform3D>(smallVase).scale = {1.f, 0.5f, 1.f};

    // Model::Builder builder{m_modelManager.cubeModelBuilder()};
    Model::Builder gameObjBuilder{};
    gameObjBuilder.loadModel("models/flat_vase.obj");

    Model::Builder cubeBuilder{};
    cubeBuilder.loadModel("models/colored_cube.obj");

    Model::Builder smoothBuilder{};
    smoothBuilder.loadModel("models/smooth_vase.obj");

    Model::Builder smallBuilder{};
    smallBuilder.loadModel("models/smooth_vase.obj");

    m_modelManager.give(gameObjBuilder, {gameObj});
    m_modelManager.give(cubeBuilder, {cube});
    m_modelManager.give(smoothBuilder, {smoothVase});
    m_modelManager.give(smallBuilder, {smallVase});
    m_modelManager.createModels();
  }

  // void Program::notifySwapchainRecreation(void* object, VkRenderPass renderPass, VkExtent2D extent)
  //{
  //   Program& program{*reinterpret_cast<Program*>(object)};
  //   program.m_renderSystem->createGraphicsPipeline(renderPass, extent);
  // }
} // namespace vke

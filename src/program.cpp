#include "program.hpp"

namespace vke
{
  Program::Program() :
      m_eventRelayer{},
      m_window{m_eventRelayer},
      m_device{m_window},
      m_modelManager{m_device},
      m_ecs{},
      m_renderer{m_device, m_window, m_eventRelayer},
      camera{}
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
    using TimeStep = std::chrono::duration<double>;
    using scTimePoint = std::chrono::steady_clock::time_point;
    using scDuration = std::chrono::steady_clock::duration;

    KeyboardInput cameraController{m_ecs};
    EntityID cameraEntity{m_ecs.createEntity()};
    m_ecs.addComponent(cameraEntity, component::Transform3D{});

    // camera.setViewDirection(glm::vec3{0.f}, glm::vec3{-0.5f, 0.0f, 1.0f});
    //    camera.setViewTarget(glm::vec3{1.f, -2.f, 3.f}, glm::vec3{0.0f, 0.0f, 2.5f});

    TimeStep timeStep;
    scTimePoint frameStartTime;
    scDuration frameDuration;

    while(!m_window.shouldClose())
    {
      frameStartTime = std::chrono::steady_clock::now();

      m_window.poolEvents();
      dispatchEvents();


      auto& translation{m_ecs.getComponent<component::Transform3D>(cameraEntity).translation};
      auto& rotation{m_ecs.getComponent<component::Transform3D>(cameraEntity).rotation};

      if(false)
      {
        system("clear");
        std::cout << "Translation\t[" << translation.x << ' ' << translation.y << ' ' << translation.z << "]" << std::endl;
        std::cout << "Rotation\t[" << rotation.x << ' ' << rotation.y << ' ' << rotation.z << "]" << std::endl;
      }

      cameraController.moveInPlaneXZ(m_window, timeStep, cameraEntity);
      // camera.setViewDirection(translation, glm::vec3{0.f, 0.f, 1.f});

      //camera.broken_setViewXYZ(translation, rotation);
      camera.setViewYXZ(translation, rotation);

      // camera.setOrthographicProjection(m_renderer.swapchainExtent(), 0.1f, 10.f);
      camera.setPerspectiveProjection(m_renderer.swapchainExtent(), glm::radians(50.f), 0.1f, 10.f);
      // camera.slow_setPerspectiveProjection(glm::radians(50.f), m_renderer.swapchainAspectRatio(), 0.1, 10.f);


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

      std::this_thread::sleep_for(std::chrono::milliseconds(32));

      frameDuration = std::chrono::steady_clock::now() - frameStartTime;
      timeStep = frameDuration;
    }

    vkDeviceWaitIdle(m_device);
  }

  void Program::dispatchEvents()
  {
    //m_eventRelayer.dispatch<event::WindowResized>();
    m_eventRelayer.dispatch<event::InvalidPipeline>();
  }

  void Program::loadEntities()
  {
    std::vector<uint32_t> indices{0, 1, 2, 2, 3, 0};
    std::vector<Model::Vertex2D> vertices2D{
      {{-0.4f, -0.4f}, {1.0f, 0.0f, 0.4f}},
      {{0.4f, -0.4f}, {0.0f, 0.4f, 1.0f}},
      {{0.4f, 0.4f}, {0.4f, 1.0f, 0.0f}},
      {{-0.4f, 0.4f}, {1.0f, 0.0f, 0.4f}}};

    m_ecs.registerComponent<component::Transform3D>();
    m_ecs.registerComponent<component::Color>();

    // auto model = std::make_shared<Model>(m_device, vertices, indices, m_renderer.swapchainImageCount());
    ModelManager::Model3DCreateInfo info{
      .vertices3d = m_modelManager.cubeModel(),
    };

//  for(int i{}; i < entityCount; ++i)
//    info.entities.push_back(m_ecs.createEntity());

    int entityCount{1};
    for(int i{}; i < entityCount; ++i)
    {
      component::Transform3D transform3D{};
      component::Color color{};

      EntityID cube{m_ecs.createEntity()};

      // square.transform2D.scale = {.4f, .6f};
      // square.transform2D.rotation = .25f * glm::two_pi<float>();
      // transform3D.translation.x = 0.f + (0.05f * i);
      // transform3D.translation.y = 0.f + (0.05f * i);

      // transform3D.rotation.x = { glm::quarter_pi<float>() };
      // transform3D.rotation.y = {glm::quarter_pi<float>()};
      // transform3D.rotation.z = {glm::quarter_pi<float>()};

      transform3D.translation.z = {2.5f};

      transform3D.scale = {.5f, .5f, .5f};
      color.color.x = 0.01f + (0.0075f * i);
      color.color.y = 0.02f + (0.0075f * i);
      color.color.z = 0.03f + (0.0075f * i);

      m_ecs.addComponent<component::Transform3D>(cube, transform3D);
      m_ecs.addComponent<component::Color>(cube, color);

      m_entities.push_back(cube);
      info.entities.push_back(cube);
    }

    m_modelManager.give(info);
    m_modelManager.createModels();
  }

  // void Program::notifySwapchainRecreation(void* object, VkRenderPass renderPass, VkExtent2D extent)
  //{
  //   Program& program{*reinterpret_cast<Program*>(object)};
  //   program.m_renderSystem->createGraphicsPipeline(renderPass, extent);
  // }
} // namespace vke

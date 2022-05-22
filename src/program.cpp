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

    m_globalDescriptorPool =
      DescriptorPool::Builder{m_device}
        .setMaxDescriptorSets(m_renderer.maxFramesInFlight())
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, m_renderer.maxFramesInFlight())
        .build();
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

    ////////// uniformBuffer //////////
    auto& limits{m_device.physicalInfo().deviceProperties.limits};
    Buffer uniformBuffer{
      m_device,
      m_renderer.maxFramesInFlight(),
      sizeof(GlobalUbo),
      VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
      0,
      std::lcm(limits.minUniformBufferOffsetAlignment, limits.nonCoherentAtomSize), // must also be aligned by nonCoherentAtomSize because of flushing
    };

    uniformBuffer.mapMemory();

    ////////// DescriptorSet //////////
    VkDescriptorSet globalDescriptorSet{};

    auto globalSetLayout =
      DescriptorSetLayout::Builder{m_device}
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
        .build();

    auto bufferInfo{uniformBuffer.descriptorInfo()};
    DescriptorWriter{*globalSetLayout, *m_globalDescriptorPool}
      .addBuffer(0, &bufferInfo)
      .allocAndUpdate(&globalDescriptorSet);

    ////////// RenderSystem //////////
    RenderSystemContext renderSystemContext{
      .eventRelayer = m_eventRelayer,
      .renderPass = m_renderer.renderPass(),
      .extent = m_renderer.swapchainExtent(),
      .globalDescriptorSetLayout = *globalSetLayout,
    };

    RenderSystem renderSystem{m_device, renderSystemContext};
    PointLightSystem pointLightSystem{m_device, renderSystemContext};

    ////////// Rendering //////////
    KeyboardInput cameraController{m_ecs, m_window};
    Camera camera{};
    EntityID cameraEntity{m_ecs.createEntity()};

    m_ecs.addComponent(cameraEntity,
      cmp::Transform3D{
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

      ////////////////
      double speed{0.3};
      for(size_t i{}; i < m_entities.size() - 1; ++i)
      {
        speed += 0.1;
        auto& eTransform{m_ecs.getComponent<cmp::Transform3D>(m_entities[i])};
        double rotX{eTransform.rotation.x + (speed * timeStep.count())};
        double rotY{eTransform.rotation.y + (speed / 2 * timeStep.count())};
        double rotZ{eTransform.rotation.z + (speed / 3 * timeStep.count())};
        eTransform.rotation.x = glm::mod(rotX, glm::two_pi<double>());
        eTransform.rotation.y = glm::mod(rotY, glm::two_pi<double>());
        eTransform.rotation.z = glm::mod(rotZ, glm::two_pi<double>());
      }
      ////////////////

      auto& translation{m_ecs.getComponent<cmp::Transform3D>(cameraEntity).translation};
      auto& rotation{m_ecs.getComponent<cmp::Transform3D>(cameraEntity).rotation};
      auto aspectRatio{m_renderer.swapchainAspectRatio()};

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
        FrameInfo info{
          .frameIndex = m_renderer.frameIndex(),
          .timeStep = timeStep,
          .commandBuffer = m_renderer.currentCommandBuffer(),
          .camera{camera},
          .ecs = m_ecs,
          .globalDescriptorSet = globalDescriptorSet,
          .entities = m_entities,
        };

        glm::vec4 cameraPos = glm::vec4(m_ecs.getComponent<cmp::Transform3D>(cameraEntity).translation, 1.0);
        GlobalUbo ubo{
          .projectionMatrix = camera.projection(),
          .ViewMatrix = camera.view(),
          .cameraPosition = cameraPos,
        };
        uniformBuffer.write(&ubo);
        uniformBuffer.flush();

        m_renderer.beginRenderPass();

        renderSystem.render(info);
        pointLightSystem.render(info);

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
      .translation{-3.f, 0.f, 1.f},
      .scale{1.f, 1.f, 1.f},
      .rotation{
        {}, // glm::radians(-15.f),
        {}, // glm::radians(35.f),
        {},
      },
    };

    cmp::Common common{};

    //  EntityID cube{m_ecs.createEntity()};
    auto flatVase = m_ecs.createEntity();
    auto cube = m_ecs.createEntity();
    auto smoothVase = m_ecs.createEntity();
    auto smallVase = m_ecs.createEntity();
    auto quad = m_ecs.createEntity();

    m_entities.push_back(flatVase);
    m_entities.push_back(smoothVase);
    m_entities.push_back(smallVase);
    m_entities.push_back(cube);
    m_entities.push_back(quad);

    for(auto& e : m_entities)
    {
      transform3D.translation.x += 0.8;

      m_ecs.addComponent<cmp::Transform3D>(e, transform3D);
      m_ecs.addComponent<cmp::Common>(e, common);
      m_ecs.addComponent<cmp::Color>(e, {});
    }

    m_ecs.getComponent<cmp::Transform3D>(smallVase).scale = {1.f, 0.5f, 1.f};
    m_ecs.getComponent<cmp::Transform3D>(quad).translation = {-0.5f, 0.5f, 1.f};
    m_ecs.getComponent<cmp::Transform3D>(quad).scale = {3.f, 1.f, 3.f};

    //Note: unexpected fragment shader behaviour when the y scale is 0.f
     /* m_ecs.getComponent<cmp::Transform3D>(quad).scale.y = 0.f; */

    // Model::Builder builder{m_modelManager.cubeModelBuilder()};
    Model::Builder flatVaseBuilder{"models/flat_vase.obj"};
    Model::Builder cubeBuilder{"models/colored_cube.obj"};
    Model::Builder smoothBuilder{"models/smooth_vase.obj"};
    Model::Builder smallBuilder{"models/smooth_vase.obj"};
    Model::Builder quadBuilder{"models/quad.obj"};

    m_modelManager.give(flatVaseBuilder, {flatVase});
    m_modelManager.give(cubeBuilder, {cube});
    m_modelManager.give(smoothBuilder, {smoothVase});
    m_modelManager.give(smallBuilder, {smallVase});
    m_modelManager.give(quadBuilder, {quad});
    m_modelManager.createModels();
  }

  // void Program::notifySwapchainRecreation(void* object, VkRenderPass renderPass, VkExtent2D extent)
  //{
  //   Program& program{*reinterpret_cast<Program*>(object)};
  //   program.m_renderSystem->createGraphicsPipeline(renderPass, extent);
  // }
} // namespace vke

#include "model.hpp"

namespace std
{
  template<>
  struct hash<vke::Model::Vertex>
  {
    std::size_t operator()(vke::Model::Vertex const& vertex) const
    {
      std::size_t seed{};
      vke::hash_combine(&seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
      return seed;
    }
  };
} // namespace std

namespace vke
{
  Model::Model(Device& device, Builder& builder) :
    m_device{device}
  {
    createVertexBuffer(builder.vertices);
    createIndexBuffer(builder.indices);
  }

  Model::~Model()
  {
  }

  void Model::createVertexBuffer(std::span<Model::Vertex> vertices)
  {
    assert(vertices.size() >= 3 && "Vertex count must be at least 3");

    auto vertexCount{static_cast<uint32_t>(vertices.size())};
    auto vertexSize{sizeof(vertices[0])};
    Buffer stagingBuffer{
      m_device,
      vertexCount,
      vertexSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    m_vertexBuffer = std::make_unique<Buffer>(
      m_device,
      vertexCount,
      vertexSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    stagingBuffer.mapMemory();
    stagingBuffer.write(vertices.data());

    // TODO: vkFlush and vkInvalidate
    // stagingBuffer.flush();

    MemAllocator::copyBuffer(m_device, stagingBuffer.handle(), m_vertexBuffer->handle(), stagingBuffer.size());
  }

  void Model::createIndexBuffer(std::span<uint32_t> indices)
  {
    m_hasIndexBuffer = indices.size() > 0;

    if(!m_hasIndexBuffer)
      return;

    auto indexCount{static_cast<uint32_t>(indices.size())};
    auto indexSize{sizeof(indices[0])};
    Buffer stagingBuffer{
      m_device,
      indexCount,
      indexSize,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    m_indexBuffer = std::make_unique<Buffer>(
      m_device,
      indexCount,
      indexSize,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    stagingBuffer.mapMemory();
    stagingBuffer.write(indices.data());

    // TODO: vkFlush and vkInvalidate
    // stagingBuffer.flush();

    MemAllocator::copyBuffer(m_device, stagingBuffer.handle(), m_indexBuffer->handle(), stagingBuffer.size());
  }

  /*
  void Model::updateUniformBuffers(uint32_t currentImage, VkExtent2D swapChainExtent)
  {
    static auto startTime{std::chrono::high_resolution_clock::now()};

    auto currentTime{std::chrono::high_resolution_clock::now()};
    float time{std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count()};

    UniformBufferObject ubo{
      .model = glm::rotate(glm::mat4(1.0f), time * glm::half_pi<float>(), glm::vec3(0.0f, 0.0f, 1.0f)),
      .view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
      .proj = glm::perspective(glm::radians(45.0f), (swapChainExtent.width / (float)swapChainExtent.height), 0.1f, 10.0f),
    };

    ubo.proj[1][1] *= -1;

    MemAllocator::mapBufferMemory(m_device, m_uniformBuffersMemory, sizeof(ubo), sizeof(UniformBufferObject) * currentImage, &ubo);
  }
  */

  void Model::bindBuffers(VkCommandBuffer commandBuffer)
  {
    assert(m_vertexBuffer && "Vertex buffer not created.");

    VkBuffer vertexBuffers[] = {m_vertexBuffer->handle()};
    VkDeviceSize offsets[]{0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    if(m_hasIndexBuffer)
      vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
  };

  void Model::draw(VkCommandBuffer commandBuffer)
  {
    // assert(!m_indicesCount && "Using a index buffer, drawIndexed() instead.");
    // assert(m_indicesCount && "Index buffer not created, draw() instead.");

    if(m_hasIndexBuffer) {
      vkCmdDrawIndexed(commandBuffer, m_indexBuffer->elementCount(), 1, 0, 0, 0);
    } else {
      vkCmdDraw(commandBuffer, m_vertexBuffer->elementCount(), 1, 0, 0);
    }
  }

  std::vector<VkVertexInputAttributeDescription> Model::Vertex::getVertexInputAttributeDescription()
  {
    return {
      {
        .location = 0, //  references the shader location
        .binding = 0,  //  which binding the per-vertex data comes
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, position),
      },
      {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, color),
      },
      {
        .location = 2,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, normal),
      },
      {
        .location = 3,
        .binding = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex, uv),
      },
    };
  }

  std::vector<VkVertexInputBindingDescription> Model::Vertex::getVertexInputBindingDescription()
  {
    // All of our per-vertex data is packed together in one array, so we're only going to have one binding
    return {
      {
        // VkVertexInputBindingDescription bindingDescription
        .binding = 0, // specifies the index of the binding in the array of bindings
        .stride = sizeof(Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
      },
    };
  }

  std::vector<VkDescriptorSetLayoutBinding> Model::UniformBufferObject::getDescriptorSetLayoutBinding()
  {
    return {
      {
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = {},
      },
    };
  }

  std::vector<VkDescriptorPoolSize> Model::UniformBufferObject::getDescriptorPoolSize(uint32_t descriptorCount)
  {
    return {
      {
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = descriptorCount,
      },
    };
  }

  void Model::Builder::loadModel(std::filesystem::path path)
  {
    if(!std::filesystem::exists(path))
      throw std::runtime_error("Invalid file path");

    tinyobj::attrib_t attrib{};             // position, color, normal and texture coordinate data
    std::vector<tinyobj::shape_t> shapes{}; // index values for each face element
    std::vector<tinyobj::material_t> materials{};
    std::string warn{}, err{};

    if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str())) {
      throw std::runtime_error("[Failed to load model]" + warn + err);
    }

    vertices.clear();
    indices.clear();

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
    for(auto const& shape : shapes) {
      for(auto const& index : shape.mesh.indices) {
        Vertex vertex{};

        // inexistent if -1
        if(index.vertex_index >= 0) {
          // each vertexertex has 3 packed values
          vertex.position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2],
          };

          // .obj doesn't support colors. This is possible through an unofficial extension for the file format, that puts RGB values after the XYZ vertex positions.

          // attrib.color is always initialized to be the same size as attrib.vertices and filled with ones wherever a color is not provided in the obj file
          vertex.color = {
            attrib.colors[3 * index.vertex_index + 0],
            attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2],
          };
        }

        if(index.normal_index >= 0) {
          vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2],
          };
        }

        if(index.texcoord_index >= 0) {
          vertex.uv = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            attrib.texcoords[2 * index.texcoord_index + 1],
          };
        }

        // For every vertex of the loaded file we check if he already exists on the map. If not then its index in the builder vertices vector is added to the map.
        if(!uniqueVertices.contains(vertex)) {
          uniqueVertices[vertex] = vertices.size();
          vertices.push_back(vertex);
        }

        // Those unique indexes are then pushed into the indices vector.
        indices.push_back(uniqueVertices[vertex]);
      }
    }
  }

///////////////////////////////////////
#if 0
  std::vector<VkVertexInputAttributeDescription> Vertex2D::getVertexInputAttributeDescription()
  {
    return {
      {
        .location = 0, //  references the shader location
        .binding = 0,  //  which binding the per-vertex data comes
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex2D, position),
      },
      {
        .location = 1,
        .binding = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex2D, color),
      },
    };
  }

  std::vector<VkVertexInputBindingDescription> Vertex2D::getVertexInputBindingDescription()
  {
    // All of our per-vertex data is packed together in one array, so we're only going to have one binding
    return {
      {
        // VkVertexInputBindingDescription bindingDescription
        .binding = 0, // specifies the index of the binding in the array of bindings
        .stride = sizeof(Vertex2D),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
      },
    };
  }

  std::vector<VkDescriptorBufferInfo> Model::getDescriptorBufferInfo()
  {
    std::vector<VkDescriptorBufferInfo> descriptorBufferInfo;
    descriptorBufferInfo.reserve(m_uniformBuffers.size());

    for(size_t i{}; i < m_uniformBuffers.size(); ++i)
    {
      VkDescriptorBufferInfo bufferInfo{
        .buffer = m_uniformBuffers[i],
        .offset = 0,
        .range = VK_WHOLE_SIZE, // sizeof(Model::UniformBufferObject),
      };

      descriptorBufferInfo.push_back(bufferInfo);
    }

    return descriptorBufferInfo;
  }

  std::vector<VkWriteDescriptorSet> Model::getWriteDescriptorSet(std::span<VkDescriptorSet> descriptorSets, std::span<VkDescriptorBufferInfo> bufferInfo)
  {
    std::vector<VkWriteDescriptorSet> descriptorWrite;
    descriptorWrite.reserve(m_uniformBuffers.size());

    for(size_t i{}; i < m_uniformBuffers.size(); ++i)
    {
      VkWriteDescriptorSet writeDescriptorSet{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSets[i],
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1, // static_cast<uint32_t>(bufferInfo.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pImageInfo = {},
        .pBufferInfo = &bufferInfo[i],
        .pTexelBufferView = {},
      };

      descriptorWrite.push_back(writeDescriptorSet);
    }

    return descriptorWrite;
  }

  void Model::createUniformBuffers(uint32_t swapChainImageCount)
  {
    throw std::runtime_error("uniform buffer creation not implemented");

    // VkDeviceSize bufferSize{sizeof(UniformBufferObject)};

    //   m_uniformBuffers.resize(swapChainImageCount);

    //    for(size_t i{}; i < swapChainImageCount; ++i)
    //      MemAllocator::createBuffer(m_device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, &m_uniformBuffers[i]);

    // constexpr VkMemoryPropertyFlags optimalProperties{VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};
    // constexpr VkMemoryPropertyFlags requiredProperties{VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    //    MemAllocator::allocateBuffers(m_device, m_uniformBuffers, m_uniformBuffersMemory, optimalProperties, requiredProperties);

    // uint32_t offset{};
    // for(size_t i{}; i < m_uniformBuffers.size(); ++i)
    //{
    //   // vkBindBufferMemory(m_device, m_uniformBuffers[i], m_uniformBuffersMemory, m_uniformBuffers[i].memoryOffset);
    //   MemAllocator::bindBufferMemory(m_device, m_uniformBuffers[i], m_uniformBuffersMemory);
    //   // offset += bufferSize;
    // }
  }

  void Model::recreateUniformBuffers(uint32_t swapChainImageCount)
  {
    throw std::runtime_error("uniform buffer recreation not implemented");
    for(VkBuffer buffer : m_uniformBuffers)
      vkDestroyBuffer(m_device, buffer, nullptr);

    vkFreeMemory(m_device, m_uniformBuffersMemory, nullptr);
    createUniformBuffers(swapChainImageCount);
  }
#endif
} // namespace vke

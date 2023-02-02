#include "vulkancore.h"
#include "VulkanglTFModel.h"

#define VERTEX_BUFFER_BIND_ID 0
#define ENABLE_VALIDATION false
#define FIELD 7
//#define OBJ_DIM 0.05f

struct Material {
	struct VulkanPC {
		float roughness;
		float metallic;
		float r;
		float g;
		float b;
	} props;
	std::string title;
	Material() {};
	Material(std::string t, glm::vec3 colour, float r, float m) : title(t) {
		props.roughness = r;
		props.metallic = m;
		props.r = colour.r;
		props.g = colour.g;
		props.b = colour.b;
	};
};

class VulkanExample : public VulkanExampleBase
{
public:
	struct Meshes {
		std::vector<vkglTF::Model> artefacts;
		int32_t artefactID = 0;
	} meshes;

	struct {
		vks::Buffer artefact;
		vks::Buffer props;
	} uniBufs;

	struct UBMs {
		glm::mat4 mapping;
		glm::mat4 mesh;
		glm::mat4 view;
		glm::vec3 camera;
	} ub_Ms;

	struct UB_Props {
		glm::vec4 lightSource[4];
	} ub_Props;

	VkPipelineLayout pl_Layout;
	VkPipeline pl;
	VkDescriptorSetLayout dSet_Layout;
	VkDescriptorSet dSet;

	//default materials to select from
	std::vector<Material> materials;
	int32_t material_ID = 0;

	std::vector<std::string> material_Title;
	std::vector<std::string> mesh_Title;

	VulkanExample() : VulkanExampleBase()
	{
		title = "Physically Based Rendering";
		camera.type = Camera::CameraType::firstperson;
		camera.setPosition(glm::vec3(10.0f, 15.0f, 1.0f));
		camera.setRotation(glm::vec3(-60.0f, 90.0f, 0.0f));
		camera.movementSpeed = 5.0f;
		camera.setPerspective(60.0f, (float)width / (float)height, 0.1f, 256.0f);
		camera.rotationSpeed = 0.3f;
		paused = true;
		timerSpeed *= 0.3f;

		//https://seblagarde.wordpress.com/2011/08/17/feeding-a-physical-based-lighting-mode/)
		//https://computergraphics.stackexchange.com/questions/4110/should-ideal-specular-multiply-light-colour-with-material-colour
		materials.push_back(Material("Iron", glm::vec3(0.56f, 0.57f, 0.58f), 0.1f, 1.0f));
		materials.push_back(Material("Copper", glm::vec3(0.95f, 0.64f, 0.54f), 0.1f, 1.0f));
		materials.push_back(Material("Gold", glm::vec3(1.0f, 0.71f, 0.29f), 0.1f, 1.0f));
		materials.push_back(Material("Aluminium", glm::vec3(0.91f, 0.92f, 0.92f), 0.1f, 1.0f));
		materials.push_back(Material("Silver", glm::vec3(0.95f, 0.93f, 0.88f), 0.1f, 1.0f));
		materials.push_back(Material("Chromium", glm::vec3(0.55f, 0.55f, 0.55f), 0.1f, 1.0f));

		for (auto material : materials) {
			material_Title.push_back(material.title);
		}
		mesh_Title = { "Sphere", "Teapot", "Suzanne", "Deer" };

		material_ID = 0;
	}

	~VulkanExample()
	{
		vkDestroyPipeline(device, pl, nullptr);

		vkDestroyPipelineLayout(device, pl_Layout, nullptr);
		vkDestroyDescriptorSetLayout(device, dSet_Layout, nullptr);

		uniBufs.artefact.destroy();
		uniBufs.props.destroy();
	}

	void createCmdBufs()
	{
		VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();

		VkClearValue cl_Vals[2];
		cl_Vals[0].color = defaultClearColor; //{ 0.4f, 0.6f, 0.9f, 1.0f };
		cl_Vals[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo rPB_Info = vks::initializers::renderPassBeginInfo();
		rPB_Info.renderPass = renderPass;
		rPB_Info.renderArea.offset.x = 0;
		rPB_Info.renderArea.offset.y = 0;
		rPB_Info.renderArea.extent.width = width;
		rPB_Info.renderArea.extent.height = height;
		rPB_Info.clearValueCount = 2;
		rPB_Info.pClearValues = cl_Vals;
		 
		for (int32_t i = 0; i < drawCmdBuffers.size(); ++i)
		{
			//set target frame buffer
			rPB_Info.framebuffer = frameBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(drawCmdBuffers[i], &cmdBufInfo));

			vkCmdBeginRenderPass(drawCmdBuffers[i], &rPB_Info, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport vp = vks::initializers::viewport((float)width, (float)height, 0.0f, 1.0f);
			vkCmdSetViewport(drawCmdBuffers[i], 0, 1, &vp);

			VkRect2D scis = vks::initializers::rect2D(width, height, 0, 0);
			vkCmdSetScissor(drawCmdBuffers[i], 0, 1, &scis);

			//objects
			vkCmdBindPipeline(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pl);
			vkCmdBindDescriptorSets(drawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pl_Layout, 0, 1, &dSet, 0, NULL);

			Material material = materials[material_ID];

			//draw materials
			for (uint32_t y = 0; y < FIELD; y++) {
				for (uint32_t x = 0; x < FIELD; x++) {
					glm::vec3 position = glm::vec3(float(x - (FIELD / 2.0f)) * 2.5f, 0.0f, float(y - (FIELD / 2.0f)) * 2.5f);
					vkCmdPushConstants(drawCmdBuffers[i], pl_Layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::vec3), &position);
					material.props.metallic = glm::clamp((float)x / (float)(FIELD - 1), 0.1f, 1.0f);
					material.props.roughness = glm::clamp((float)y / (float)(FIELD - 1), 0.05f, 1.0f);
					vkCmdPushConstants(drawCmdBuffers[i], pl_Layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::vec3), sizeof(Material::VulkanPC), &material);
					meshes.artefacts[meshes.artefactID].draw(drawCmdBuffers[i]);
				}
			}

			drawUI(drawCmdBuffers[i]);

			vkCmdEndRenderPass(drawCmdBuffers[i]);

			VK_CHECK_RESULT(vkEndCommandBuffer(drawCmdBuffers[i]));
		}
	}
	
	void loadAssets()
	{
		std::vector<std::string> files = { "sphere.gltf", "teapot.gltf", "suzanne.gltf", "deer.gltf" };
		meshes.artefacts.resize(files.size());
		for (size_t i = 0; i < files.size(); i++) {			
			meshes.artefacts[i].loadFromFile(getAssetPath() + "models/" + files[i], vulkanDevice, queue, vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::FlipY);
		}
	}

	void setupDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> dsl_Binding = {
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0),
			vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
		};

		VkDescriptorSetLayoutCreateInfo dsl_Info =
			vks::initializers::descriptorSetLayoutCreateInfo(dsl_Binding);

		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &dsl_Info, nullptr, &dSet_Layout));

		VkPipelineLayoutCreateInfo plLayout_Info =
			vks::initializers::pipelineLayoutCreateInfo(&dSet_Layout, 1);

		std::vector<VkPushConstantRange> pC_Range = {
			vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::vec3), 0),
			vks::initializers::pushConstantRange(VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(Material::VulkanPC), sizeof(glm::vec3)),
		};

		plLayout_Info.pushConstantRangeCount = 2;
		plLayout_Info.pPushConstantRanges = pC_Range.data();

		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &plLayout_Info, nullptr, &pl_Layout));
	}

	void setupDescriptorSets()
	{
		//Descriptor Pool
		std::vector<VkDescriptorPoolSize> pool_Size = {
			vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4),
		};

		VkDescriptorPoolCreateInfo dP_Info =
			vks::initializers::descriptorPoolCreateInfo(pool_Size, 2);

		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &dP_Info, nullptr, &descriptorPool));

		//Descriptor sets

		VkDescriptorSetAllocateInfo dSA_Info =
			vks::initializers::descriptorSetAllocateInfo(descriptorPool, &dSet_Layout, 1);

		//3D object descriptor set
		VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &dSA_Info, &dSet));

		std::vector<VkWriteDescriptorSet> write_DSet = {
			vks::initializers::writeDescriptorSet(dSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniBufs.artefact.descriptor),
			vks::initializers::writeDescriptorSet(dSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, &uniBufs.props.descriptor),
		};
		vkUpdateDescriptorSets(device, static_cast<uint32_t>(write_DSet.size()), write_DSet.data(), 0, NULL);
	}

	void preparePipelines()
	{
		VkPipelineInputAssemblyStateCreateInfo iA_State =  vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
		VkPipelineRasterizationStateCreateInfo rast_State = vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineColorBlendAttachmentState blendA_State = vks::initializers::pipelineColorBlendAttachmentState(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo cB_State = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendA_State);
		VkPipelineDepthStencilStateCreateInfo depSten_State = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo vpState = vks::initializers::pipelineViewportStateCreateInfo(1, 1);
		VkPipelineMultisampleStateCreateInfo ms_State = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
		std::vector<VkDynamicState> dynSt_Enable = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynSt_Enable);
		VkGraphicsPipelineCreateInfo plC_Info = vks::initializers::pipelineCreateInfo(pl_Layout, renderPass);

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
		plC_Info.pInputAssemblyState = &iA_State;
		plC_Info.pRasterizationState = &rast_State;
		plC_Info.pColorBlendState = &cB_State;
		plC_Info.pMultisampleState = &ms_State;
		plC_Info.pViewportState = &vpState;
		plC_Info.pDepthStencilState = &depSten_State;
		plC_Info.pDynamicState = &dynamicState;
		plC_Info.stageCount = static_cast<uint32_t>(shaderStages.size());
		plC_Info.pStages = shaderStages.data();
		plC_Info.pVertexInputState = vkglTF::Vertex::getPipelineVertexInputState({ vkglTF::VertexComponent::Position, vkglTF::VertexComponent::Normal });

		//PBR pipeline
		shaderStages[0] = loadShader(getShadersPath() + "pbrbasic/pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
		shaderStages[1] = loadShader(getShadersPath() + "pbrbasic/pbr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
		//Enable depth test and write
		depSten_State.depthWriteEnable = VK_TRUE;
		depSten_State.depthTestEnable = VK_TRUE;
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &plC_Info, nullptr, &pl));
	}

	//Prepare and initialize uniform buffer containing shader uniforms
	void prepareUniformBuffers()
	{
		//Object vertex shader uniform buffer
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&uniBufs.artefact,
			sizeof(ub_Ms)));

		//Shared parameter uniform buffer
		VK_CHECK_RESULT(vulkanDevice->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&uniBufs.props,
			sizeof(ub_Props)));

		//Map persistent
		VK_CHECK_RESULT(uniBufs.artefact.map());
		VK_CHECK_RESULT(uniBufs.props.map());

		updateUniformBuffers();
		updateLights();
	}

	void updateUniformBuffers()
	{
		//3D object
		ub_Ms.mapping = camera.matrices.perspective;
		ub_Ms.view = camera.matrices.view;
		ub_Ms.mesh = glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f + (meshes.artefactID == 1 ? 45.0f : 0.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
		ub_Ms.camera = camera.position * -1.0f;
		memcpy(uniBufs.artefact.mapped, &ub_Ms, sizeof(ub_Ms));
	}

	void updateLights()
	{
		const float pos = 15.0f;
		ub_Props.lightSource[0] = glm::vec4(-pos, -pos*0.5f, -pos, 1.0f);
		ub_Props.lightSource[1] = glm::vec4(-pos, -pos*0.5f,  pos, 1.0f);
		ub_Props.lightSource[2] = glm::vec4( pos, -pos*0.5f,  pos, 1.0f);
		ub_Props.lightSource[3] = glm::vec4( pos, -pos*0.5f, -pos, 1.0f);
		
		//not relevant for now
		if (!paused)
		{
			ub_Props.lightSource[0].x = sin(glm::radians(timer * 360.0f)) * 20.0f;
			ub_Props.lightSource[1].x = cos(glm::radians(timer * 360.0f)) * 20.0f;
			ub_Props.lightSource[0].z = cos(glm::radians(timer * 360.0f)) * 20.0f;
			ub_Props.lightSource[1].y = sin(glm::radians(timer * 360.0f)) * 20.0f;
		}

		memcpy(uniBufs.props.mapped, &ub_Props, sizeof(ub_Props));
	}

	void draw()
	{
		VulkanExampleBase::prepareFrame();

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &drawCmdBuffers[currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

		VulkanExampleBase::submitFrame();
	}

	void prepare()
	{
		VulkanExampleBase::prepare();
		loadAssets();
		prepareUniformBuffers();
		setupDescriptorSetLayout();
		preparePipelines();
		setupDescriptorSets();
		createCmdBufs();
		prepared = true;
	}

	virtual void render()
	{
		if (!prepared)
			return;
		draw();
		if (!paused)
			updateLights();
	}

	virtual void viewChanged()
	{
		updateUniformBuffers();
	}

	virtual void OnUpdateUIOverlay(vks::UIOverlay *overlay)
	{
		if (overlay->header("Setup")) {
			if (overlay->comboBox("Selected Material", &material_ID, material_Title)) {
				createCmdBufs();
			}
			if (overlay->comboBox("Selected Mesh", &meshes.artefactID, mesh_Title)) {
				updateUniformBuffers();
				createCmdBufs();
			}
		}
	}
};

VULKAN_EXAMPLE_MAIN()
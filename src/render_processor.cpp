#include "../toy2d/render_processor.hpp"
#include "../toy2d/shader.hpp"
#include "../toy2d/context.hpp"
//#include "../toy2d/vertex.hpp"
#include "../toy2d/math.hpp"
namespace toy2d
{
    RenderProcess::RenderProcess() {
        layout = createLayout();
        renderPass = createRenderPass();
        graphicsPipeline = nullptr;
    }

    RenderProcess::~RenderProcess() {
        auto& ctx = Context::Instance();
        ctx.device.destroyRenderPass(renderPass);
        ctx.device.destroyPipelineLayout(layout);
        ctx.device.destroyPipeline(graphicsPipeline);
    }

    void RenderProcess::RecreateGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource) {
        if (graphicsPipeline) {
            Context::Instance().device.destroyPipeline(graphicsPipeline);
        }
        graphicsPipeline = createGraphicsPipeline(vertexSource, fragSource);
    }

    void RenderProcess::RecreateRenderPass() {
        if (renderPass) {
            Context::Instance().device.destroyRenderPass(renderPass);
        }
        renderPass = createRenderPass();
    }

    vk::PipelineLayout RenderProcess::createLayout() {
        vk::PipelineLayoutCreateInfo createInfo;
        createInfo.setPushConstantRangeCount(0)
            .setSetLayoutCount(0);

        return Context::Instance().device.createPipelineLayout(createInfo);
    }

    vk::Pipeline RenderProcess::createGraphicsPipeline(const std::vector<char>& vertexSource, const std::vector<char>& fragSource) {
        auto& ctx = Context::Instance();
        auto& device = ctx.device;
		vk::GraphicsPipelineCreateInfo createInfo; // 图形管线创建信息
		//1. create shader module
        const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStateCreateInfos = Shader::GetInstance().GetStages();

       

        // 0. shader prepare
        /*vk::ShaderModuleCreateInfo vertexModuleCreateInfo, fragModuleCreateInfo;
        vertexModuleCreateInfo.codeSize = vertexSource.size();
        vertexModuleCreateInfo.pCode = (std::uint32_t*)vertexSource.data();
        fragModuleCreateInfo.codeSize = fragSource.size();
        fragModuleCreateInfo.pCode = (std::uint32_t*)fragSource.data();

        auto vertexModule = ctx.device.createShaderModule(vertexModuleCreateInfo);
        auto fragModule = ctx.device.createShaderModule(fragModuleCreateInfo);

        std::array<vk::PipelineShaderStageCreateInfo, 2> stageCreateInfos;
        stageCreateInfos[0].setModule(vertexModule)
            .setPName("main")
            .setStage(vk::ShaderStageFlagBits::eVertex);
        stageCreateInfos[1].setModule(fragModule)
            .setPName("main")
            .setStage(vk::ShaderStageFlagBits::eFragment);*/

        // 2. vertex input
		vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
        auto attribute = Vec::GetAttributeDescriptions();
		auto binding = Vec::GetBindingDescriptions();
        vertexInputCreateInfo.setVertexAttributeDescriptions( attribute )
            .setVertexBindingDescriptions(binding);
      
        /*vk::PipelineVertexInputStateCreateInfo vertexInputCreateInfo;
        auto attribute = Vertex::getAttribute();
        auto binding = Vertex::getBinding();
        vertexInputCreateInfo.setVertexAttributeDescriptions(attribute);
        vertexInputCreateInfo.setVertexBindingDescriptions(binding);*/

        // 3. vertex assembly
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
        inputAssemblyCreateInfo.setPrimitiveRestartEnable(false)
            .setTopology(vk::PrimitiveTopology::eTriangleList);
            

        /*vk::PipelineInputAssemblyStateCreateInfo inputAsmCreateInfo;
        inputAsmCreateInfo.setPrimitiveRestartEnable(false)
            .setTopology(vk::PrimitiveTopology::eTriangleList);*/

        

        // 4. viewport and scissor
        /*vk::PipelineViewportStateCreateInfo viewportInfo;
        vk::Viewport viewport(0, 0, ctx.swapchain->GetExtent().width, ctx.swapchain->GetExtent().height, 0, 1);
        vk::Rect2D scissor(vk::Rect2D({ 0, 0 }, ctx.swapchain->GetExtent()));
        viewportInfo.setViewports(viewport)
            .setScissors(scissor);*/

        vk::PipelineViewportStateCreateInfo viewportStateCreateInfo;
        vk::Viewport viewport(0.0f, 0.0f,
            static_cast<float>(ctx.swapchain->GetExtent().width),
            static_cast<float>(ctx.swapchain->GetExtent().height),
            0.f, 1.f);

		vk::Rect2D rect({ 0, 0 }, ctx.swapchain->GetExtent());
        viewportStateCreateInfo.setViewportCount(1)
            .setViewports(viewport)
            .setScissorCount(1)
            .setScissors(rect);
        //动态设置viewport 和 scissor
        /*
        vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo;
        std::array<vk::DynamicState, 2> aryDynamicState = { vk::DynamicState::eViewport,
        vk::DynamicState::eScissor};
        

        dynamicStateCreateInfo.setDynamicStates(aryDynamicState);*/

        // 5. rasteraizer
        /*vk::PipelineRasterizationStateCreateInfo rasterInfo;
        rasterInfo.setCullMode(vk::CullModeFlagBits::eFront)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthClampEnable(false)
            .setLineWidth(1)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setRasterizerDiscardEnable(false);*/

		vk::PipelineRasterizationStateCreateInfo rasterStateCreateInfo;
        rasterStateCreateInfo.setPolygonMode(vk::PolygonMode::eFill)
            .setCullMode(vk::CullModeFlagBits::eFront)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setLineWidth(1.0f)
			.setDepthClampEnable(false)
			.setRasterizerDiscardEnable(false);
      
        // 7. multisampler
       /* vk::PipelineMultisampleStateCreateInfo multisampleInfo;
        multisampleInfo.setSampleShadingEnable(false)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);*/

		vk::PipelineMultisampleStateCreateInfo multisampleCreateInfo;
        multisampleCreateInfo.setSampleShadingEnable(false)
            .setRasterizationSamples(vk::SampleCountFlagBits::e1);
        // 8. depth and stencil buffer
        // We currently don't need depth and stencil buffer

        // 9. blending
        /*vk::PipelineColorBlendAttachmentState blendAttachmentState;
        blendAttachmentState.setBlendEnable(false)
            .setColorWriteMask(vk::ColorComponentFlagBits::eA |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eR);
        vk::PipelineColorBlendStateCreateInfo blendInfo;
        blendInfo.setAttachments(blendAttachmentState)
            .setLogicOpEnable(false);*/


        
		vk::PipelineColorBlendAttachmentState colorBlendAttachmentState;
        colorBlendAttachmentState.setBlendEnable(false)
            .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA);
        vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo;
        colorBlendStateCreateInfo.setAttachments(colorBlendAttachmentState)
            .setLogicOpEnable(false);
            
        //createInfo.setPColorBlendState(&colorBlendStateCreateInfo)

        // create graphics pipeline
        /*createInfo.setStages(stageCreateInfos)
            .setLayout(layout)
            .setPVertexInputState(&vertexInputCreateInfo)
            .setPInputAssemblyState(&inputAsmCreateInfo)
            .setPViewportState(&viewportInfo)
            .setPRasterizationState(&rasterInfo)
            .setPMultisampleState(&multisampleInfo)
            .setPColorBlendState(&blendInfo)
            .setRenderPass(renderPass);*/
        createInfo.setPVertexInputState(&vertexInputCreateInfo)
            .setPInputAssemblyState(&inputAssemblyCreateInfo)
            .setStages(shaderStateCreateInfos)
            .setPViewportState(&viewportStateCreateInfo)
            .setPRasterizationState(&rasterStateCreateInfo)
            .setPMultisampleState(&multisampleCreateInfo)
            .setPColorBlendState(&colorBlendStateCreateInfo)
            .setLayout(this->layout)
            //.setPDynamicState(&dynamicStateCreateInfo) //当viewport大小动态调整时，使用
           
            .setRenderPass(renderPass);
        
            
            //.setStageCount(static_cast<std::uint32_t>(shaderStateCreateInfos.size()));

        auto result = ctx.device.createGraphicsPipeline(nullptr, createInfo);
        if (result.result != vk::Result::eSuccess) {
            std::cout << "create graphics pipeline failed: " << result.result << std::endl;
        }

       

        return result.value;
    }

    vk::RenderPass RenderProcess::createRenderPass() {
        auto& ctx = Context::Instance();

		

       /* vk::RenderPassCreateInfo createInfo;

        vk::SubpassDependency dependency;
        dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
            .setDstSubpass(0)
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

        vk::AttachmentDescription attachDescription;
        attachDescription.setFormat(ctx.swapchain->GetFormat().format)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
            .setInitialLayout(vk::ImageLayout::eUndefined)
            .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
        vk::AttachmentReference reference;
        reference.setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        vk::SubpassDescription subpassDesc;
        subpassDesc.setColorAttachments(reference)
            .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);

        
        createInfo.setAttachments(attachDescription)
            .setDependencies(dependency)
            .setSubpasses(subpassDesc);*/

       // return Context::Instance().device.createRenderPass(createInfo);

        //1.定义附件描述
		//1.1 定义颜色附件描述
		vk::AttachmentDescription colorAttachmentDescription;  //附件描述
		auto format = ctx.swapchain->GetFormat().format; // 获取交换链图像格式
        colorAttachmentDescription.setFormat(format)
            .setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)  // 加载操作：清除
			.setStoreOp(vk::AttachmentStoreOp::eStore)// 存储操作：存储
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)  //模板加载操作：不关心
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)//  模板存储操作：不关心
			.setInitialLayout(vk::ImageLayout::eUndefined)// 初始布局：未定义         
			.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);// 最终布局：呈现源KHR
        
		//二维用不到深度附件
		/*vk::AttachmentDescription depthAttachmentDescription;
        depthAttachmentDescription.setFormat(vk::Format::eD32Sfloat)
            .setSamples(vk::SampleCountFlagBits::e1)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
            .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);*/


        vk::AttachmentReference colorAttachmentReference; //颜色附件引用
        colorAttachmentReference.setAttachment(0)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal); // 布局：颜色附件最佳

        //二维用不到深度附件
		/*vk::AttachmentReference depthAttachmentReference;
        depthAttachmentReference.setAttachment(1)
			.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);*/
		//2.定义子通道描述
		

		vk::SubpassDescription subpassDescription; //子通道描述
        subpassDescription.setColorAttachments(colorAttachmentReference)
            /*.setInputAttachments()
            .setResolveAttachments()*/
			.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics); // 管线绑定点：图形


		//3.定义子通道依赖
		vk::SubpassDependency subpassDependency; //子通道依赖
		subpassDependency.setSrcSubpass(VK_SUBPASS_EXTERNAL) // 源子通道：外部
			.setDstSubpass(0) // 目标子通道：0
            .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
            .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
           	.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite); // 目标访问掩码：颜色附件写入
        
        vk::RenderPassCreateInfo renderPassCreateInfo;
        renderPassCreateInfo.setAttachments(colorAttachmentDescription)
           .setSubpasses(subpassDescription)
            .setDependencies(subpassDependency);
        

		return ctx.device.createRenderPass(renderPassCreateInfo);
    }
}
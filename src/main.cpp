#include <iostream>
#include <print>

#include <istream>
#include <sstream>
#include <fstream>

#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "core/application.h"
#include "renderer/debugRenderer.h"
#include "renderer/renderer.h"

struct NeuronNode {
    int32_t id;
    int32_t parent_id;
    glm::vec3 position;
    float radius;
    uint32_t type;
};

static bool parse_swc_line(const std::string& line, NeuronNode& node) {
    if (line.empty() || line.starts_with('#')) {
        return false;
    }

    std::istringstream neuron_line_stream(line);

    neuron_line_stream >> node.id
                       >> node.type
                       >> node.position.x
                       >> node.position.y
                       >> node.position.z
                       >> node.radius
                       >> node.parent_id;

    return true;
}

static bool read_swc(std::string file_name, std::vector<NeuronNode>& nodes) {
    std::ifstream input_file(file_name);
    std::string swc_content;

    if (!input_file.is_open()) {
        std::println(stderr, "Error opening input file {}", file_name);
        return false;
    }

    input_file.seekg(0, std::ios::end);
    swc_content.reserve(input_file.tellg());
    input_file.seekg(0, std::ios::beg);

    swc_content.assign((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

    nodes.reserve(std::count(swc_content.begin(), swc_content.end(), '\n'));

    std::istringstream stream(swc_content);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\n') {
            line.pop_back();
        }

        if (NeuronNode node{}; parse_swc_line(line, node)) {
            nodes.emplace_back(node);
        }
    }

    return true;
}

static DG::ApplicationSettings viewerSettings {
    .windowWidth = 1280,
    .windowHeight = 720,
    .windowTitle = "SWC Neuron Viewer"
};

class ViewerLayer : public DG::Layer {
private:
    std::vector<NeuronNode> m_nodes;
    std::unordered_map<int32_t, NeuronNode*> m_nodeMap; //NOTE: For finding parentNode

    float m_minX = 0.0f, m_minY = 0.0f;
    float m_scale = 1.0f;
    float m_padding = 40.0f;

    DG::Camera m_camera;

public:
    ViewerLayer() {
        DG::Renderer2D::Init();

        if (!read_swc("assets/dros-melan.CNG.swc", m_nodes)) {
            return;
        }

        m_minY = 1e9f; float maxY = -1e9f;
        m_minX = 1e9f; float maxX = -1e9f;
        for (auto& node : m_nodes) {
            m_nodeMap[node.id] = &node;

            m_minY = std::min(m_minY, node.position.y);
            maxY = std::max(maxY, node.position.y);

            m_minX = std::min(m_minX, node.position.x);
            maxX = std::max(maxX, node.position.x);
        }
        const float neuronHeight = maxY - m_minY;
        const float neuronWidth = maxX - m_minX;

        float screenW = static_cast<float>(viewerSettings.windowWidth);
        float screenH = static_cast<float>(viewerSettings.windowHeight);

        m_scale = std::min((screenW - 2.0f * m_padding) / neuronWidth, (screenH - 2.0f * m_padding) / neuronHeight);

        m_camera = DG::Camera::Create2D(screenW, screenH, 720.0f);
    };

    void OnAttach() override {};
    void OnDettach() override {};

    void OnUpdate(float deltaTime) override {
        DG::Renderer2D::Clear(0.0f, 0.5f, 0.5f, 1.0f);

        DG::Renderer2D::BeginScene(m_camera);

        DrawNeuronConnections();
        //DrawNeuronNodes();

        DG::Renderer2D::EndScene();
    };

    //TODO: Should pre calc neuron positions, then recalculate on camera movement. So maybe a camera on matrix changed callback of bool?
    void DrawNeuronNodes() const {
        for (const NeuronNode &node : m_nodes) {
            DG::DebugRenderer::DrawFilledCircle(calculateNeuronPosition(node), 1.0f, {0.3f, 0.3f, 1.0f, 1.0f});
        }
    }

    void DrawNeuronConnections() {
        for (auto& node : m_nodes) {
            if (node.parent_id == -1) continue; //NOTE: -1 is the root node, so has no parent connection

            const auto currentNodePos = calculateNeuronPosition(node);
            const auto parentNodePos = calculateNeuronPosition(*m_nodeMap[node.parent_id]);

            DG::DebugRenderer::DrawLine(currentNodePos, parentNodePos, {1.0f, 0.3f, 0.5f, 1.0f});
        };
    }


    void OnGuiDraw() override {};

private:
    [[nodiscard]] glm::vec2 calculateNeuronPosition(const NeuronNode& node) const {
        return {
            m_padding + (node.position.x - m_minX) * m_scale,
            m_padding + (node.position.y - m_minY) * m_scale
        };
    }

};

class NeuronViewer : public DG::Application {
public:
    NeuronViewer() : DG::Application(viewerSettings) {
        std::println("Initializing SWC NeuronViewer");

        PushLayer(std::make_unique<ViewerLayer>());
    }
};

int main() {
    NeuronViewer viewer;
    viewer.Run();

    return 0;
}

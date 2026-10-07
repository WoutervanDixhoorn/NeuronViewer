#include <iostream>
#include <print>

#include <istream>
#include <sstream>
#include <fstream>

#include <glm/glm.hpp>

struct NeuronNode {
    int32_t id;
    int32_t parent_id;
    glm::vec3 position;
    float radius; //NOTE: This actually a decimal number mostly below zero. Example: 0.64 -> 64
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

    nodes.reserve(std::count(swc_content.begin(), swc_content.end(), '\r'));

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


int main() {
    std::println("Reading swc file...");

    std::vector<NeuronNode> nodes;
    if (!read_swc("assets/dros-melan.CNG.swc", nodes)) {
        return 1;
    }

    std::println("Reading swc file finished!");

    std::println("Amount of neuron nodes: {}", nodes.size());

    return 0;
}

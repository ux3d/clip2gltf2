#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

bool loadFile(std::string& output, const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	file.seekg(0);

	output.resize(fileSize);

	file.read(output.data(), fileSize);
	file.close();

	return true;
}

bool saveFile(const std::string& output, const std::string& filename)
{
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open())
	{
		return false;
	}

	file.write(output.data(), output.size());
	file.close();

	return true;
}

void generateScale(json& glTF, std::vector<float>& floatData, const size_t rows, const size_t columns, const float duration, const float epsilon)
{
	size_t clips = rows * columns;

	// Nodes and meshes
	size_t newBuffer = 3;

	for (size_t i = 0; i < clips; i++)
	{
		glTF["nodes"].push_back(json::object());
		glTF["nodes"][i + 1]["mesh"] = i;
		glTF["nodes"][i + 1]["scale"] = json::array();

		if (i > 0)
		{
			glTF["nodes"][i + 1]["scale"][0] = epsilon;
			glTF["nodes"][i + 1]["scale"][1] = epsilon;
			glTF["nodes"][i + 1]["scale"][2] = epsilon;

			glTF["meshes"].push_back(glTF["meshes"][0]);

			glTF["animations"][0]["channels"].push_back(glTF["animations"][0]["channels"][0]);
			glTF["animations"][0]["samplers"].push_back(glTF["animations"][0]["samplers"][0]);
		}
		else
		{
			glTF["nodes"][i + 1]["scale"][0] = 1.0f;
			glTF["nodes"][i + 1]["scale"][1] = 1.0f;
			glTF["nodes"][i + 1]["scale"][2] = 1.0f;

			// Mesh does already exist

			// Channel does already exist

			// Sampler does already exist
		}

		glTF["meshes"][i]["primitives"][0]["attributes"]["TEXCOORD_0"] = newBuffer + i;

		glTF["nodes"][0]["children"].push_back(i + 1);

		glTF["animations"][0]["channels"][i]["sampler"] = i;
		glTF["animations"][0]["channels"][i]["target"]["node"] = i + 1;
	}

	// Frames

	size_t index = newBuffer;

	size_t byteOffset = 108;
	size_t byteLength = 0;

	float columnsStep = 1.0f / (float)columns;
	float rowsStep = 1.0f / (float)rows;

	for (size_t row = 0; row < rows; row++)
	{
		for (size_t column = 0; column < columns; column++)
		{
			// UV for Vertex 0
			floatData.push_back(glm::clamp(columnsStep * (float)(column + 0), 0.0f, 1.0f));
			floatData.push_back(glm::clamp(rowsStep * (float)(row + 0), 0.0f, 1.0f));
			// UV for Vertex 1
			floatData.push_back(glm::clamp(columnsStep * (float)(column + 0), 0.0f, 1.0f));
			floatData.push_back(glm::clamp(rowsStep * (float)(row + 1), 0.0f, 1.0f));

			// UV for Vertex 2
			floatData.push_back(glm::clamp(columnsStep * (float)(column + 1), 0.0f, 1.0f));
			floatData.push_back(glm::clamp(rowsStep * (float)(row + 0), 0.0f, 1.0f));
			// UV for Vertex 3
			floatData.push_back(glm::clamp(columnsStep * (float)(column + 1), 0.0f, 1.0f));
			floatData.push_back(glm::clamp(rowsStep * (float)(row + 1), 0.0f, 1.0f));

			//

			byteLength = sizeof(float) * 8;

			auto bufferView = json::object();
			bufferView["buffer"] = 0;
			bufferView["byteLength"] = byteLength;
			bufferView["byteOffset"] = byteOffset;
			byteOffset += byteLength;

			glTF["bufferViews"].push_back(bufferView);

			auto accessor = json::object();
			accessor["bufferView"] = index;
			accessor["byteOffset"] = 0;
			accessor["componentType"] = 5126;
			accessor["count"] = 4;
			accessor["normalized"] = false;
			accessor["type"] = "VEC2";

			glTF["accessors"].push_back(accessor);

			index++;
		}
	}

	// Time as input

	float frameTime = duration / (float)clips;

	for (size_t i = 0; i < clips; i++)
	{
		floatData.push_back((float)i * frameTime);

		glTF["animations"][0]["samplers"][i]["input"] = glTF["accessors"].size();
	}
	// Storing two values
	byteLength = sizeof(float) * clips;

	auto accessor = json::object();
	auto bufferView = json::object();

	bufferView["buffer"] = 0;
	bufferView["byteLength"] = byteLength;
	bufferView["byteOffset"] = byteOffset;

	accessor["bufferView"] = glTF["bufferViews"].size();
	accessor["componentType"] = 5126;
	accessor["count"] = clips;
	accessor["min"] = json::array();
	accessor["min"][0] = 0.0;
	accessor["max"] = json::array();
	accessor["max"][0] = floatData.back() + frameTime;	// Last frame should not switch immediately
	accessor["type"] = "SCALAR";

	glTF["bufferViews"].push_back(bufferView);
	glTF["accessors"].push_back(accessor);

	byteOffset += byteLength;

	// Scale as output

	for (size_t i = 0; i < clips; i++)
	{
		for (size_t k = 0; k < clips; k++)
		{
			if (i == k)
			{
				floatData.push_back(1.0f);
				floatData.push_back(1.0f);
				floatData.push_back(1.0f);
			}
			else
			{
				floatData.push_back(epsilon);
				floatData.push_back(epsilon);
				floatData.push_back(epsilon);
			}
		}
		byteLength = sizeof(float) * 3 * clips;

		accessor = json::object();
		bufferView = json::object();

		bufferView["buffer"] = 0;
		bufferView["byteLength"] = byteLength;
		bufferView["byteOffset"] = byteOffset;

		accessor["bufferView"] = glTF["bufferViews"].size();
		accessor["componentType"] = 5126;
		accessor["count"] = clips;
		accessor["type"] = "VEC3";

		glTF["animations"][0]["samplers"][i]["output"] = glTF["accessors"].size();

		glTF["bufferViews"].push_back(bufferView);
		glTF["accessors"].push_back(accessor);

		byteOffset += byteLength;
	}
}

void generateTranslation(json& glTF, std::vector<float>& floatData, const size_t rows, const size_t columns, const float duration, const float epsilon)
{
	size_t clips = rows * columns;

	// Nodes and meshes
	size_t newBuffer = 3;

	for (size_t i = 0; i < clips; i++)
	{
		glTF["nodes"].push_back(json::object());
		glTF["nodes"][i + 1]["mesh"] = i;
		glTF["nodes"][i + 1]["translation"] = json::array();

		if (i > 0)
		{
			glTF["nodes"][i + 1]["translation"][0] = epsilon;
			glTF["nodes"][i + 1]["translation"][1] = epsilon;
			glTF["nodes"][i + 1]["translation"][2] = epsilon;

			glTF["meshes"].push_back(glTF["meshes"][0]);

			glTF["animations"][0]["channels"].push_back(glTF["animations"][0]["channels"][0]);
			glTF["animations"][0]["samplers"].push_back(glTF["animations"][0]["samplers"][0]);
		}
		else
		{
			glTF["nodes"][i + 1]["translation"][0] = 0.0f;
			glTF["nodes"][i + 1]["translation"][1] = 0.0f;
			glTF["nodes"][i + 1]["translation"][2] = 0.0f;

			// Mesh does already exist

			// Channel does already exist

			// Sampler does already exist
		}

		glTF["meshes"][i]["primitives"][0]["attributes"]["TEXCOORD_0"] = newBuffer + i;

		glTF["nodes"][0]["children"].push_back(i + 1);

		glTF["animations"][0]["channels"][i]["sampler"] = i;
		glTF["animations"][0]["channels"][i]["target"]["node"] = i + 1;
	}

	// Frames

	size_t index = newBuffer;

	size_t byteOffset = 108;
	size_t byteLength = 0;

	float columnsStep = 1.0f / (float)columns;
	float rowsStep = 1.0f / (float)rows;

	for (size_t row = 0; row < rows; row++)
	{
		for (size_t column = 0; column < columns; column++)
		{
			// UV for Vertex 0
			floatData.push_back(glm::clamp(columnsStep * (float)(column + 0), 0.0f, 1.0f));
			floatData.push_back(glm::clamp(rowsStep * (float)(row + 0), 0.0f, 1.0f));
			// UV for Vertex 1
			floatData.push_back(glm::clamp(columnsStep * (float)(column + 0), 0.0f, 1.0f));
			floatData.push_back(glm::clamp(rowsStep * (float)(row + 1), 0.0f, 1.0f));

			// UV for Vertex 2
			floatData.push_back(glm::clamp(columnsStep * (float)(column + 1), 0.0f, 1.0f));
			floatData.push_back(glm::clamp(rowsStep * (float)(row + 0), 0.0f, 1.0f));
			// UV for Vertex 3
			floatData.push_back(glm::clamp(columnsStep * (float)(column + 1), 0.0f, 1.0f));
			floatData.push_back(glm::clamp(rowsStep * (float)(row + 1), 0.0f, 1.0f));

			//

			byteLength = sizeof(float) * 8;

			auto bufferView = json::object();
			bufferView["buffer"] = 0;
			bufferView["byteLength"] = byteLength;
			bufferView["byteOffset"] = byteOffset;
			byteOffset += byteLength;

			glTF["bufferViews"].push_back(bufferView);

			auto accessor = json::object();
			accessor["bufferView"] = index;
			accessor["byteOffset"] = 0;
			accessor["componentType"] = 5126;
			accessor["count"] = 4;
			accessor["normalized"] = false;
			accessor["type"] = "VEC2";

			glTF["accessors"].push_back(accessor);

			index++;
		}
	}

	// Time as input

	float frameTime = duration / (float)clips;

	for (size_t i = 0; i < clips; i++)
	{
		floatData.push_back((float)i * frameTime);

		glTF["animations"][0]["samplers"][i]["input"] = glTF["accessors"].size();
	}
	// Storing two values
	byteLength = sizeof(float) * clips;

	auto accessor = json::object();
	auto bufferView = json::object();

	bufferView["buffer"] = 0;
	bufferView["byteLength"] = byteLength;
	bufferView["byteOffset"] = byteOffset;

	accessor["bufferView"] = glTF["bufferViews"].size();
	accessor["componentType"] = 5126;
	accessor["count"] = clips;
	accessor["min"] = json::array();
	accessor["min"][0] = 0.0;
	accessor["max"] = json::array();
	accessor["max"][0] = floatData.back() + frameTime;	// Last frame should not switch immediately
	accessor["type"] = "SCALAR";

	glTF["bufferViews"].push_back(bufferView);
	glTF["accessors"].push_back(accessor);

	byteOffset += byteLength;

	// Translation as output

	for (size_t i = 0; i < clips; i++)
	{
		for (size_t k = 0; k < clips; k++)
		{
			if (i == k)
			{
				floatData.push_back(0.0f);
				floatData.push_back(0.0f);
				floatData.push_back(0.0f);
			}
			else
			{
				floatData.push_back(epsilon);
				floatData.push_back(epsilon);
				floatData.push_back(epsilon);
			}
		}
		// Storing two vector values
		byteLength = sizeof(float) * 3 * clips;

		accessor = json::object();
		bufferView = json::object();

		bufferView["buffer"] = 0;
		bufferView["byteLength"] = byteLength;
		bufferView["byteOffset"] = byteOffset;

		accessor["bufferView"] = glTF["bufferViews"].size();
		accessor["componentType"] = 5126;
		accessor["count"] = clips;
		accessor["type"] = "VEC3";

		glTF["animations"][0]["samplers"][i]["output"] = glTF["accessors"].size();

		glTF["bufferViews"].push_back(bufferView);
		glTF["accessors"].push_back(accessor);

		byteOffset += byteLength;
	}
}

void generateMorph(json& glTF, std::vector<float>& floatData, const size_t rows, const size_t columns, const float duration)
{
	size_t clips = rows * columns;

	// Weights and targets

	glTF["meshes"][0]["weights"].push_back(1.0f);
	glTF["meshes"][0]["primitives"][0]["targets"].push_back(json::object());
	size_t index = 6;
	glTF["meshes"][0]["primitives"][0]["targets"][0]["TEXCOORD_0"] = index;				// Needs to be created
	for (size_t i = 1; i < clips; i++)
	{
		glTF["meshes"][0]["weights"].push_back(0.0f);
		glTF["meshes"][0]["primitives"][0]["targets"].push_back(json::object());
		glTF["meshes"][0]["primitives"][0]["targets"][i]["TEXCOORD_0"] = index + i;		// Needs to be created
	}

	// Time as input

	float frameTime = duration / (float)clips;

	size_t byteOffset = 140;

	for (size_t i = 0; i < clips; i++)
	{
		floatData.push_back((float)i * frameTime);
	}

	size_t byteLength = sizeof(float) * clips;

	glTF["bufferViews"][4]["byteLength"] = byteLength;
	glTF["accessors"][4]["count"] = clips;
	glTF["accessors"][4]["max"][0] = floatData.back() + frameTime;

	byteOffset += byteLength;

	// Weights as output

	for (size_t i = 0; i < clips; i++)
	{
		for (size_t k = 0; k < clips; k++)
		{
			if (i == k)
			{
				floatData.push_back(1.0f);
			}
			else
			{
				floatData.push_back(0.0f);
			}
		}
	}

	byteLength = sizeof(float) * clips * clips;

	glTF["bufferViews"][5]["byteOffset"] = byteOffset;
	glTF["bufferViews"][5]["byteLength"] = byteLength;
	glTF["accessors"][5]["count"] = clips * clips;

	byteOffset += byteLength;

	// Frames

	float columnsStep = 1.0f / (float)columns;
	float rowsStep = 1.0f / (float)rows;

	for (size_t row = 0; row < rows; row++)
	{
		for (size_t column = 0; column < columns; column++)
		{
			// UV for Vertex 0
			floatData.push_back(columnsStep * (float)column);
			floatData.push_back(rowsStep * (float)row);
			// UV for Vertex 1
			floatData.push_back(columnsStep * (float)column);
			floatData.push_back(rowsStep * (float)row);

			// UV for Vertex 2
			floatData.push_back(-1.0f + columnsStep * (float)(column + 1));
			floatData.push_back(-1.0f + rowsStep * (float)(row + 1));
			// UV for Vertex 3
			floatData.push_back(-1.0f + columnsStep * (float)(column + 1));
			floatData.push_back(-1.0f + rowsStep * (float)(row + 1));

			//

			byteLength = sizeof(float) * 8;

			auto bufferView = json::object();
			bufferView["buffer"] = 0;
			bufferView["byteLength"] = byteLength;
			bufferView["byteOffset"] = byteOffset;
			byteOffset += byteLength;

			glTF["bufferViews"].push_back(bufferView);

			auto accessor = json::object();
			accessor["bufferView"] = index;
			accessor["byteOffset"] = 0;
			accessor["componentType"] = 5126;
			accessor["count"] = 4;
			accessor["normalized"] = false;
			accessor["type"] = "VEC2";

			glTF["accessors"].push_back(accessor);

			index++;
		}
	}
}

int main(int argc, char *argv[])
{
	size_t rows = 1;
	size_t columns = 6;
	float duration = 1.0f;
	std::string imageName = "RunningGirl.png";
	size_t mode = 0;
	float epsilon = 0.001f;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-r") == 0 && (i + 1 < argc))
        {
            rows = (size_t)std::stoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-c") == 0 && (i + 1 < argc))
        {
            columns = (size_t)std::stoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-d") == 0 && (i + 1 < argc))
        {
            duration = (float)std::stof(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-i") == 0 && (i + 1 < argc))
        {
        	imageName = argv[i + 1];
        }
        else if (strcmp(argv[i], "-m") == 0 && (i + 1 < argc))
        {
            mode = (size_t)std::stoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-e") == 0 && (i + 1 < argc))
        {
            epsilon = (float)std::stof(argv[i + 1]);
        }
    }

	//
	//

	std::string loadBinaryName = "scale.bin";
	if (mode == 1)
	{
		loadBinaryName = "translation.bin";
	}
	else if (mode == 2)
	{
		loadBinaryName = "morph.bin";
	}
	std::string saveBinaryName = "untitled.bin";

	std::string binaryContent;
	if (!loadFile(binaryContent, loadBinaryName))
	{
		printf("Error: Could not load binary file '%s'\n", loadBinaryName.c_str());

		return -1;
	}

	std::string loadTemplateName = "scale.gltf";
	if (mode == 1)
	{
		loadTemplateName = "translation.gltf";
	}
	else if (mode == 2)
	{
		loadTemplateName = "morph.gltf";
	}
	std::string saveTemplateName = "untitled.gltf";

    std::string templateContent;
	if (!loadFile(templateContent, loadTemplateName))
	{
		printf("Error: Could not load template file '%s'\n", loadTemplateName.c_str());

		return -1;
	}

	//
	//

	json glTF = json::parse(templateContent);

	//

	std::vector<float> floatData;

	if (mode == 0)
	{
		generateScale(glTF, floatData, rows, columns, duration, epsilon);
	}
	else if (mode == 1)
	{
		generateTranslation(glTF, floatData, rows, columns, duration, 10000.0f);
	}
	else if (mode == 2)
	{
		generateMorph(glTF, floatData, rows, columns, duration);
	}
	else
	{
		printf("Error: Invalid mode %llu\n", mode);

		return -1;
	}

	//

    std::string data;
    // Make space for existing and new content
    data.resize(floatData.size() * sizeof(float) + binaryContent.size());
    // Copy current content
    memcpy(data.data(), binaryContent.data(), binaryContent.size());
    // Copy new content with existing content as offset
    memcpy(data.data() + binaryContent.size(), floatData.data(), floatData.size() * sizeof(float));

    //
    //

	if (!saveFile(data, saveBinaryName))
	{
		printf("Error: Could not save generated bin file '%s'\n", saveBinaryName.c_str());

		return -1;
	}

	glTF["buffers"][0]["uri"] = saveBinaryName;
	glTF["buffers"][0]["byteLength"] = data.size();

	glTF["images"][0]["uri"] = imageName;

	if (!saveFile(glTF.dump(3), saveTemplateName))
	{
		printf("Error: Could not save generated glTF file '%s'\n", saveTemplateName.c_str());

		return -1;
	}

	printf("Info: Saved glTF '%s'\n", saveTemplateName.c_str());

	return 0;
}

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

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

int main(int argc, char *argv[])
{
	size_t rows = 1;
	size_t columns = 6;
	float fps = 15.0f;
	std::string imageName = "pngegg.png";

    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-r") == 0 && (i + 1 < argc))
        {
            rows = (size_t)std::stoi(argv[i + 1]);
        }
        if (strcmp(argv[i], "-c") == 0 && (i + 1 < argc))
        {
            columns = (size_t)std::stoi(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-f") == 0 && (i + 1 < argc))
        {
            fps = (float)std::stof(argv[i + 1]);
        }
        else if (strcmp(argv[i], "-i") == 0 && (i + 1 < argc))
        {
        	imageName = argv[i + 1];
        }
    }

	size_t clips = rows * columns;

	//
	//

	std::string loadBinaryName = "morph.bin";
	std::string saveBinaryName = "untitled.bin";

	std::string binaryContent;
	if (!loadFile(binaryContent, loadBinaryName))
	{
		printf("Error: Could not load binary file '%s'\n", loadBinaryName.c_str());

		return -1;
	}

	std::string loadTemplateName = "morph.gltf";
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

	size_t byteOffset = 140;

	for (size_t i = 0; i < clips; i++)
	{
		floatData.push_back(i * (float)(1.0f / fps));
	}

	size_t byteLength = sizeof(float) * clips;

	glTF["bufferViews"][4]["byteLength"] = byteLength;
	glTF["accessors"][4]["count"] = clips;
	glTF["accessors"][4]["max"][0] = floatData.back();

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

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

	std::string loadBinaryName = "original.bin";
	std::string saveBinaryName = "untitled.bin";

	std::string binaryContent;
	if (!loadFile(binaryContent, loadBinaryName))
	{
		printf("Error: Could not load binary file '%s'\n", loadBinaryName.c_str());

		return -1;
	}

	std::string loadTemplateName = "original.gltf";
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

	std::vector<float> floatData;

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
		}
	}

	// Time

	for (size_t i = 0; i < clips; i++)
	{
		floatData.push_back(i * (float)(1.0f / fps));
	}

	glTF["accessors"][10]["max"][0] = floatData.back();

	// Weights

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
	glTF["images"][0]["uri"] = imageName;

	if (!saveFile(glTF.dump(3), saveTemplateName))
	{
		printf("Error: Could not save generated glTF file '%s'\n", saveTemplateName.c_str());

		return -1;
	}

	return 0;
}

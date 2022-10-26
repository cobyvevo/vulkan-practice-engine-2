#pragma once

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <sstream>

namespace MeshTools {

	struct MeshVertex{
		float pos[3] = {0,0,0};
		float uv[2] = {0,0};

		void print() {
			std::cout << "{pos : " << pos[0] << "," << pos[1] << "," << pos[2] << " - UV: " << uv[0] << "," << uv[1] << "}" << std::endl;
		}
	};

	struct MeshData {
		std::vector<MeshVertex> vertices;
		std::vector<int> indices;

		bool load_from_file(const char* path) {

			std::ifstream meshfile;
			meshfile.open(path);
			if (meshfile.fail()) return false;

			std::string line;

			std::vector<std::array<float,2>> UV_storage;

			while (!meshfile.eof()) {
				meshfile >> line; 
				std::cout << "reading" << line << std::endl;

				if (line == "v") {
					
					std::cout << "new vertex" << std::endl;

					MeshVertex vert;
					for (int i = 0; i < 3; i++){
						meshfile >> line; 
						std::cout << "float:" << line << std::endl;
						vert.pos[i] = std::stof(line);
					}
					vertices.push_back(vert);

				} else if (line == "vt") {
					std::cout << "new UV" << std::endl;

					//MeshVertex* vert = &vertices[UV_index];
					std::array<float,2> uv;

					for (int i = 0; i < 2; i++){
						meshfile >> line; 
						std::cout << "float:" << line << std::endl;
						uv[i] = std::stof(line);
					}

					UV_storage.push_back(uv);
					//UV_index++;

				} else if (line == "f") {
					std::cout << "face index" << std::endl;

					for (int i = 0; i < 3; i++){
						meshfile >> line; 

						int vertex_stri = line.find("/", 0); //stri = string index
						int uv_stri = line.find("/", vertex_stri+1);

						std::string vertexindex_str = line.substr(0,vertex_stri);
						std::string uvindex_str = line.substr(vertex_stri+1,uv_stri-vertex_stri-1);

						std::cout << "vertex: " << vertexindex_str << "uvindex:" << uvindex_str << std::endl;

						int vertexindex = std::stoi(vertexindex_str)-1;
						int uvindex = std::stoi(uvindex_str)-1;

						vertices[vertexindex].uv[0] = UV_storage[uvindex][0];
						vertices[vertexindex].uv[1] = UV_storage[uvindex][1];

						indices.push_back(vertexindex);					
					}

				}

			}

			UV_storage.clear();

			for (int indic : indices) {
				vertices[indic].print();
			}

			return true;
		};

	};

}
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
		float norm[3] = {0,0,0};

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
			std::vector<std::array<float,3>> vertices_storage;
			std::vector<std::array<float,3>> normal_storage;

			int index_count = 0;

			while (!meshfile.eof()) {
				meshfile >> line; 
				std::cout << "reading" << line << std::endl;

				if (line == "v") {
					
					std::cout << "new vertex" << std::endl;

					std::array<float,3>  vert;
					for (int i = 0; i < 3; i++){
						meshfile >> line; 
						std::cout << "float:" << line << std::endl;
						vert[i] = std::stof(line);
					}
					vertices_storage.push_back(vert);
				} else if (line == "vn") {
					std::cout << "new VN" << std::endl;
					std::array<float,3>  normal;
					for (int i = 0; i < 3; i++){
						meshfile >> line; 
						std::cout << "float:" << line << std::endl;
						normal[i] = std::stof(line);
					}
					normal_storage.push_back(normal);
				} else if (line == "vt") {
					std::cout << "new UV" << std::endl;
					std::array<float,2> uv;

					for (int i = 0; i < 2; i++){
						meshfile >> line; 
						std::cout << "float:" << line << std::endl;
						uv[i] = std::stof(line);
					}

					UV_storage.push_back(uv);
				} else if (line == "f") {
					std::cout << "face index" << std::endl;

					for (int i = 0; i < 3; i++){
						meshfile >> line; 

						int vertex_stri = line.find("/", 0); //stri = string index
						int uv_stri = line.find("/", vertex_stri+1);
						int norm_stri = line.find("/", uv_stri+1);

						std::string vertexindex_str = line.substr(0,vertex_stri);
						std::string uvindex_str = line.substr(vertex_stri+1,uv_stri-vertex_stri-1);
						std::string normindex_str = line.substr(uv_stri+1,norm_stri-uv_stri-1);

						std::cout << "vertex: " << vertexindex_str << "uvindex:" << uvindex_str << "normalindex:" << normindex_str << std::endl;

						int vertexindex = std::stoi(vertexindex_str)-1;
						int uvindex = std::stoi(uvindex_str)-1;
						int normindex = std::stoi(normindex_str)-1;

						MeshVertex newvert;

						newvert.uv[0] = UV_storage[uvindex][0];
						newvert.uv[1] = UV_storage[uvindex][1];

						newvert.pos[0] = vertices_storage[vertexindex][0];
						newvert.pos[1] = vertices_storage[vertexindex][1];
						newvert.pos[2] = vertices_storage[vertexindex][2];

						newvert.norm[0] = normal_storage[normindex][0];
						newvert.norm[1] = normal_storage[normindex][1];
						newvert.norm[2] = normal_storage[normindex][2];

						vertices.push_back(newvert);
						indices.push_back(index_count);
						index_count++;					
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
#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <entt/entt.hpp>
#include <memory>

class Scene;

class _Entity {
	public:
		Entity(){};
		Entity(unsigned int mesh, unsigned int material, unsigned int state, unsigned int program=0) :
							mesh(mesh), material(material), state(state), program(program), hasTexture(false) {};
		unsigned int material 	= 0;
		unsigned int mesh 		= 0;
		unsigned int state 		= 0;
		unsigned int program 	= 0;
		unsigned int texture 	= 0;
		bool hasTexture = false;
};


class Entity {
	private:
		entt::entity entity_handle = entt::null;
		std::make_shared<Scene> scene;

	public:
		Entity(entt::entity s_entity_handle, Scene* s_scene);

		inline entt::entity get_handle() { 
			return entity_handle; 
			}

		template<typename T, typename... Args>
		T& add_component(Args&&... args){
			return scene->registry.emplace<T>(entity_handle, std::forward<Args>(args)...);
		}

		template<typename T>
		T& get_component(){
			return scene->registry.get<T>(entity_handle);
		}

		template<typename T>
		void remove_component(){
			return scene->registry.remove<T>(entity_handle);
		}

		template<typename T>
		bool has_component(){
			return scene->registry.all_of<T>(entity_handle);
		}
};

#endif

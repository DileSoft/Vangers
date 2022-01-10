//
// Created by nikita on 26.11.2021.
//

#ifndef VANGERS_RENDERINGCONTEXT_H
#define VANGERS_RENDERINGCONTEXT_H

#include <memory>
#include <cassert>

#include "AbstractRenderer.h"


namespace renderer::scene {
	class RenderingContext {
	public:
		static void create(std::unique_ptr<AbstractRenderer>&& renderer) {
			_instance = std::make_unique<RenderingContext>(std::move(renderer));
		}

		static bool has_renderer() {
			return (bool)_instance;
		}

		static void reset(){
			_instance.reset();
		}

//		static std::unique_ptr<RenderingContext>& instance() {
//			return _instance;
//		}

		static inline std::unique_ptr<AbstractRenderer>& renderer() {
			assert(_instance && "RenderingContext instance is not created");

			return _instance->_renderer;
		}

		// TODO: private
		explicit RenderingContext(std::unique_ptr<AbstractRenderer>&& renderer){
			_renderer = std::move(renderer);
		}

	private:
		std::unique_ptr<AbstractRenderer> _renderer;
		static std::unique_ptr<RenderingContext> _instance;
	};
}




#endif //VANGERS_RENDERINGCONTEXT_H

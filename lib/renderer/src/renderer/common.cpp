#include "common.h"

std::ostream &operator<<(std::ostream &os, const renderer::Rect &rect){
	return os << "Rect{" 
	 << ".x=" << rect.x
	 << ", .y=" << rect.y
	 << ", .width=" << rect.width
	 << ", .height=" << rect.height
	 << "}";
}

std::ostream &operator<<(std::ostream &os, const renderer::Color &color){
	return os << "Color{" 
	 << ".r=" << color.r
	 << ", .g=" << color.g
	 << ", .b=" << color.b
	 << ", .a=" << color.a
	 << "}";
}
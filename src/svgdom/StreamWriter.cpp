#include "StreamWriter.hpp"

#include <utki/util.hpp>

#include "util.hxx"
#include "Exc.hpp"

using namespace svgdom;

void StreamWriter::setName(const std::string& name) {
	this->name = name;
}

void StreamWriter::addAttribute(const std::string& name, const std::string& value) {
	this->attributes.push_back(std::make_pair(name, value));
}

void StreamWriter::addAttribute(const std::string& name, const Length& value){
	std::stringstream ss;
	ss << value;
	this->addAttribute(name, ss.str());
}

void StreamWriter::addAttribute(const std::string& name, real value){
	std::stringstream ss;
	ss << value;
	this->addAttribute(name, ss.str());
}

void StreamWriter::write(const Container* children) {
	auto ind = indentStr();

	auto tag = std::move(this->name);
	
	this->s << ind << "<" << tag;
	
	for(auto& a : this->attributes){
		this->s << " " << a.first << "=\"" << a.second << "\"";
	}

	this->attributes.clear();
	this->name.clear();
	
	if (!children || children->children.size() == 0) {
		this->s << "/>";
	} else {
		this->s << ">" << std::endl;
		this->childrenToStream(*children);
		this->s << ind << "</" << tag << ">";
	}
	this->s << std::endl;
}

std::string StreamWriter::indentStr() {
	std::string ind;

	std::stringstream ss;
	for(unsigned i = 0; i != this->indent; ++i){
		ss << "\t";
	}
	return ss.str();
}

void StreamWriter::childrenToStream(const Container& e) {
	++this->indent;
	utki::ScopeExit scopeExit([this]() {
		--this->indent;
	});
	for (auto& c : e.children) {
		c->accept(*this);
	}
}

void StreamWriter::addElementAttributes(const Element& e) {
	if(e.id.length() != 0){
		this->addAttribute("id", e.id);
	}
}

void StreamWriter::addTransformableAttributes(const Transformable& e) {
	if(e.transformations.size() != 0){
		this->addAttribute("transform", e.transformationsToString());
	}
}

void StreamWriter::addStyleableAttributes(const Styleable& e) {
	if(e.styles.size() != 0){
		this->addAttribute("style", e.stylesToString());
	}
}

void StreamWriter::addViewBoxedAttributes(const ViewBoxed& e) {
	if (e.isViewBoxSpecified()) {
		this->addAttribute("viewBox", e.viewBoxToString());
	}

	if (e.preserveAspectRatio.preserve != ViewBoxed::PreserveAspectRatio_e::NONE || e.preserveAspectRatio.defer || e.preserveAspectRatio.slice) {
		this->addAttribute("preserveAspectRatio", e.preserveAspectRatioToString());
	}
}

void StreamWriter::addRectangleAttributes(const Rectangle& e) {
	if(e.x.value != 0){
		this->addAttribute("x", e.x);
	}
	
	if(e.y.value != 0){
		this->addAttribute("y", e.y);
	}
	
	if(e.width.value != 100 || e.width.unit != Length::Unit_e::PERCENT){ //if width is not 100% (default value)
		this->addAttribute("width", e.width);
	}
	
	if(e.height.value != 100 || e.height.unit != Length::Unit_e::PERCENT){ //if height is not 100% (default value)
		this->addAttribute("height", e.height);
	}
}

void StreamWriter::addReferencingAttributes(const Referencing& e) {
	if(e.iri.length() != 0){
		this->addAttribute("xlink:href", e.iri);
	}
}

void StreamWriter::addGradientAttributes(const Gradient& e) {
	this->addElementAttributes(e);
	this->addReferencingAttributes(e);
	this->addStyleableAttributes(e);
	
	if(e.spreadMethod != Gradient::SpreadMethod_e::DEFAULT){
		this->addAttribute("spreadMethod", e.spreadMethodToString());
	}
	
	if(e.units != CoordinateUnits_e::UNKNOWN && e.units != CoordinateUnits_e::OBJECT_BOUNDING_BOX){
		this->addAttribute("gradientUnits", coordinateUnitsToString(e.units));
	}
	
	if(e.transformations.size() != 0){
		this->addAttribute("gradientTransform", e.transformationsToString());
	}
}


void StreamWriter::addShapeAttributes(const Shape& e) {
	this->addElementAttributes(e);
	this->addTransformableAttributes(e);
	this->addStyleableAttributes(e);
}


void StreamWriter::visit(const GElement& e) {
	this->setName("g");
	this->addElementAttributes(e);
	this->addTransformableAttributes(e);
	this->addStyleableAttributes(e);
	this->write(&e);
}

void StreamWriter::visit(const SvgElement& e) {
	this->setName("svg");
	
	if(this->indent == 0){//if outermost "svg" element
		this->addAttribute("xmlns", "http://www.w3.org/2000/svg");
		this->addAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
		this->addAttribute("version", "1.1");
	}
	
	this->addElementAttributes(e);
	this->addStyleableAttributes(e);
	this->addRectangleAttributes(e);
	this->addViewBoxedAttributes(e);
	this->write(&e);
}

void StreamWriter::visit(const LineElement& e) {
	this->setName("line");
	this->addShapeAttributes(e);
	
	if(e.x1.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("x1", e.x1);
	}
	
	if(e.y1.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("y1", e.y1);
	}
	
	if(e.x2.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("x2", e.x2);
	}
	
	if(e.y2.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("y2", e.y2);
	}
	
	this->write();
}

void StreamWriter::visit(const RectElement& e) {
	this->setName("rect");
	this->addShapeAttributes(e);
	this->addRectangleAttributes(e);
	
	if(e.rx.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("rx", e.rx);
	}
	
	if(e.ry.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("ry", e.ry);
	}
	
	this->write();
}

void StreamWriter::visit(const EllipseElement& e) {
	this->setName("ellipse");
	this->addShapeAttributes(e);
	
	if(e.cx.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("cx", e.cx);
	}
	
	if(e.cy.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("cy", e.cy);
	}
	
	if(e.rx.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("rx", e.rx);
	}
	
	if(e.ry.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("ry", e.ry);
	}
	
	this->write();
}

void StreamWriter::visit(const PolygonElement& e) {
	this->setName("polygon");
	this->addShapeAttributes(e);
	if(e.points.size() != 0){
		this->addAttribute("points", e.pointsToString());
	}
	this->write();
}

void StreamWriter::visit(const PolylineElement& e) {
	this->setName("polyline");
	this->addShapeAttributes(e);
	if(e.points.size() != 0){
		this->addAttribute("points", e.pointsToString());
	}
	this->write();
}

void StreamWriter::visit(const CircleElement& e) {
	this->setName("circle");
	this->addShapeAttributes(e);
	
	if(e.cx.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("cx", e.cx);
	}
	
	if(e.cy.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("cy", e.cy);
	}
	
	if(e.r.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("r", e.r);
	}
	this->write();
}

void StreamWriter::visit(const PathElement& e){
	this->setName("path");
	this->addShapeAttributes(e);
	if(e.path.size() != 0){
		this->addAttribute("d", e.pathToString());
	}
	this->write();
}

void StreamWriter::visit(const UseElement& e) {
	this->setName("use");
	this->addElementAttributes(e);
	this->addTransformableAttributes(e);
	this->addStyleableAttributes(e);
	this->addRectangleAttributes(e);
	this->addReferencingAttributes(e);
	this->write();
}

void StreamWriter::visit(const Gradient::StopElement& e) {
	this->setName("stop");
	this->addAttribute("offset", e.offset);
	this->addElementAttributes(e);
	this->addStyleableAttributes(e);
	this->write();
}

void StreamWriter::visit(const RadialGradientElement& e) {
	this->setName("radialGradient");
	this->addGradientAttributes(e);
	if(e.cx.unit != Length::Unit_e::PERCENT || e.cx.value != 50){
		this->addAttribute("cx", e.cx);
	}
	if(e.cy.unit != Length::Unit_e::PERCENT || e.cy.value != 50){
		this->addAttribute("cy", e.cy);
	}
	if(e.r.unit != Length::Unit_e::PERCENT || e.r.value != 50){
		this->addAttribute("r", e.r);
	}
	if(e.fx.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("fx", e.fx);
	}
	if(e.fy.unit != Length::Unit_e::UNKNOWN){
		this->addAttribute("fy", e.fy);
	}
	this->write(&e);
}

void StreamWriter::visit(const LinearGradientElement& e) {
	this->setName("linearGradient");
	this->addGradientAttributes(e);
	if(e.x1.unit != Length::Unit_e::PERCENT || e.x1.value != 0){
		this->addAttribute("x1", e.x1);
	}
	if(e.y1.unit != Length::Unit_e::PERCENT || e.y1.value != 0){
		this->addAttribute("y1", e.y1);
	}
	if(e.x2.unit != Length::Unit_e::PERCENT || e.x2.value != 100){
		this->addAttribute("x2", e.x2);
	}
	if(e.y2.unit != Length::Unit_e::PERCENT || e.y2.value != 0){
		this->addAttribute("y2", e.y2);
	}
	this->write(&e);
}

void StreamWriter::visit(const DefsElement& e) {
	this->setName("defs");
	this->addElementAttributes(e);
	this->addTransformableAttributes(e);
	this->addStyleableAttributes(e);
	this->write(&e);
}

void StreamWriter::visit(const SymbolElement& e) {
	this->setName("symbol");
	this->addElementAttributes(e);
	this->addStyleableAttributes(e);
	this->addViewBoxedAttributes(e);
	this->write(&e);
}

void StreamWriter::visit(const FilterElement& e){
	this->setName("filter");
	this->addElementAttributes(e);
	this->addStyleableAttributes(e);
	this->addRectangleAttributes(e);
	this->addReferencingAttributes(e);
	
	if(e.filterUnits != CoordinateUnits_e::UNKNOWN && e.filterUnits != CoordinateUnits_e::OBJECT_BOUNDING_BOX){
		this->addAttribute("filterUnits", coordinateUnitsToString(e.filterUnits));
	}

	if(e.primitiveUnits != CoordinateUnits_e::UNKNOWN && e.primitiveUnits != CoordinateUnits_e::USER_SPACE_ON_USE){
		this->addAttribute("primitiveUnits", coordinateUnitsToString(e.primitiveUnits));
	}
	
	this->write(&e);
}

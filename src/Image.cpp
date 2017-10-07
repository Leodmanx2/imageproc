#include "Image.hpp"

Dimensions::Dimensions(unsigned int w, unsigned int h) : width(w), height(h) {}

Image::Image(const Dimensions& dimensions, unsigned int channels)
  : m_dimensions(dimensions), m_channels(channels) {
	m_data.reserve(dimensions.width * dimensions.height * channels);
}

Image::Image(const std::string& filename) : m_dimensions(0, 0) {
	OIIO::ImageInput* input = OIIO::ImageInput::open(filename);
	if(!input) {
		std::stringstream ss;
		ss << "cannot open file " << filename << "\n";
		throw std::runtime_error(ss.str());
	}

	const OIIO::ImageSpec& spec      = input->spec();
	const int&             xres      = spec.width;
	const int&             yres      = spec.height;
	const int&             nchannels = spec.nchannels;

	m_dimensions.width  = xres;
	m_dimensions.height = yres;
	m_channels          = nchannels;

	m_data.resize(xres * yres * nchannels);

	input->read_image(
	  {OIIO::TypeDesc::UINT8, OIIO::TypeDesc::SCALAR, OIIO::TypeDesc::COLOR},
	  m_data.data());

	input->close();
	OIIO::ImageInput::destroy(input);
}

void Image::save(const std::string& filename) const {
	OIIO::ImageOutput* out = OIIO::ImageOutput::create(filename);
	OIIO::ImageSpec    spec(
    m_dimensions.width,
    m_dimensions.height,
    m_channels,
    {OIIO::TypeDesc::UINT8, OIIO::TypeDesc::SCALAR, OIIO::TypeDesc::COLOR});
	out->open(filename, spec);
	out->write_image(
	  {OIIO::TypeDesc::UINT8, OIIO::TypeDesc::SCALAR, OIIO::TypeDesc::COLOR},
	  m_data.data());
	out->close();
	delete out;
}

#ifndef IMAGE_H
#define IMAGE_H

#include "gsl-lite.hpp"
#include <OpenImageIO/imageio.h>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

using namespace gsl;

struct Dimensions final {
	unsigned int width;
	unsigned int height;

	Dimensions(unsigned int w, unsigned int h);
};

class Image final {
	private:
	Dimensions           m_dimensions;
	std::vector<uint8_t> m_data;
	unsigned int         m_channels;

	public:
	explicit Image(const Dimensions& dimensions, unsigned int channels);
	explicit Image(const std::string& filename);

	inline uint8_t
	at(unsigned int x, unsigned int y, unsigned int channel) const {
		Expects(x < m_dimensions.width);
		Expects(y < m_dimensions.height);
		Expects(channel < m_channels);
		const index index = m_channels * (y * m_dimensions.width + x);
		return m_data.at(index + channel);
	}

	inline const Dimensions& dimensions() const { return m_dimensions; }

	inline unsigned int channels() const { return m_channels; }

	inline void
	set(unsigned int x, unsigned int y, uint8_t value, unsigned int channel) {
		Expects(x < m_dimensions.width);
		Expects(y < m_dimensions.height);
		Expects(channel < m_channels);
		const index index = m_channels * (y * m_dimensions.width + x) + channel;
		m_data[index]     = value;
	}

	void save(const std::string& filename) const;
};

#endif

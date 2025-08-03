#ifndef VPT_IMAGE_HPP
#define VPT_IMAGE_HPP

#include <Eigen/Dense>

#include <filesystem>

namespace vpt {

namespace detail {
  template <typename T>
  struct supports_save {
    constexpr static bool value = 
        std::is_integral_v<T>
        && std::is_unsigned_v<T>
        && (sizeof(T) == 1 || sizeof(T) == 2);
  };

  template <typename T>
  inline constexpr bool supports_save_v = supports_save<T>::value;

  bool save_image(const std::filesystem::path& path, const char* data, int width, int height, int color_channels, int bit_depth);
}

using image_index_t = Eigen::Index;
using image_point_t = Eigen::Vector2<image_index_t>;

struct image_size_t {
  image_index_t width;
  image_index_t height;
};

struct image_rect_t {
  image_point_t start;
  image_size_t size;
};

template <typename T, size_t N_channels>
struct Image {
  using value_t = Eigen::Vector<T, N_channels>;
  
  Image(size_t width, size_t height) { m_data.resize(height, width); }

  inline image_size_t size() const { return { m_data.cols(), m_data.rows() }; }

  inline auto data()       { return decltype(m_data)::Map(m_data.data(), m_data.rows(), m_data.cols()); }
  inline auto data() const { return decltype(m_data)::Map(m_data.data(), m_data.rows(), m_data.cols()); }

  inline auto view(const image_rect_t& r) { return data().block(r.start.y(), r.start.x(), r.size.height, r.size.width); }

  std::enable_if_t<detail::supports_save_v<T>, bool> save(const std::filesystem::path& path) {
    return detail::save_image(path, reinterpret_cast<char*>(m_data.data()->data()), size().width, size().height, N_channels, 8 * sizeof(T));
  }

private:
  Eigen::Matrix<value_t, -1, -1, Eigen::DontAlign | Eigen::RowMajor> m_data;
};

} // namespace vpt

#endif // !VPT_IMAGE_HPP
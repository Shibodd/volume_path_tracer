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

template <typename T, size_t N_channels>
struct Image {
  using value_type = Eigen::Vector<T, N_channels>;
  
  Image(size_t width, size_t height) { m_data.resize(width, height);}

  inline auto data()       { return decltype(m_data)::Map(m_data.data(), m_data.rows(), m_data.cols()); }
  inline auto data() const { return decltype(m_data)::Map(m_data.data(), m_data.rows(), m_data.cols()); }

  std::enable_if_t<detail::supports_save_v<T>, bool> save(const std::filesystem::path& path) {
    return detail::save_image(path, reinterpret_cast<char*>(m_data.data()->data()), m_data.cols(), m_data.rows(), N_channels, 8 * sizeof(T));
  }

private:
  Eigen::Matrix<value_type, -1, -1> m_data;
};

} // namespace vpt

#endif // !VPT_IMAGE_HPP
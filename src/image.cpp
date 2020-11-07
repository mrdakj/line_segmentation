#include "image.h" 
#include <cassert>

// image
image::image(const fs::path& path)
    : m_data((fs::exists(path)) ? cv::imread(path.string(), cv::IMREAD_GRAYSCALE) : (throw std::runtime_error("File not found."), cv::Mat()))
{
}

image::image(int height, int width)
    : m_data(cv::Mat(height, width, CV_8UC1, cv::Scalar((int)Color::white)))
{
}

image::image(const image& img, int left, int right)
    : m_data(img.mat()(cv::Range::all(), cv::Range(left, right)))
{
}

image::image(cv::Mat data)
    : m_data(std::move(data))
{
}

int image::rows() const
{
    return m_data.rows;
}

int image::cols() const
{
    return m_data.cols;
}

void image::crop()
{
    auto [h, w] = dim();
    auto is_entry_black = [](const auto& x) { return x(0,0) == (int)Color::black; };

    auto top_it = std::find_if(cvit::row_iterator(*this), cvit::row_iterator(), [&](const auto& row) {
        return std::any_of(cvit::column_iterator(row), cvit::column_iterator(), is_entry_black);
    });
    int top = (top_it != cvit::row_iterator()) ? std::distance(cvit::row_iterator(*this), top_it) : -1;

    auto bottom_it = std::find_if(cvit::row_iterator_r(*this, h-1), cvit::row_iterator_r(), [&](const auto& row) {
        return std::any_of(cvit::column_iterator(row), cvit::column_iterator(), is_entry_black);
    });
    int bottom = (bottom_it != cvit::row_iterator_r()) ? std::distance(bottom_it, cvit::row_iterator_r(*this,0)) : -1;

    auto left_it = std::find_if(cvit::column_iterator(*this), cvit::column_iterator(), [&](const auto& col) {
        return std::any_of(cvit::row_iterator(col), cvit::row_iterator(), is_entry_black);
    });
    int left = (left_it != cvit::column_iterator()) ? std::distance(cvit::column_iterator(*this), left_it) : -1;

    auto right_it = std::find_if(cvit::column_iterator_r(*this, w-1), cvit::column_iterator_r(), [&](const auto& col) {
        return std::any_of(cvit::row_iterator(col), cvit::row_iterator(), is_entry_black);
    });
    int right = (right_it != cvit::column_iterator_r()) ? std::distance(right_it, cvit::column_iterator_r(*this, 0)) : -1;

    if (left == -1 || right == -1 || top == -1 || bottom == -1) {
        // cannot crop an empty picture
        return;
    }

    borders b{left, right+1, top, bottom+1};
    m_data = m_data({b.left(), b.top(), b.width(), b.height()});
}

void image::add_border()
{
    cv::copyMakeBorder(m_data, m_data, 1, 1, 1, 1, cv::BORDER_CONSTANT, (int)Color::white);
}

std::pair<int,int> image::dim() const
{
    return {m_data.rows, m_data.cols};
}

const cv::Mat& image::mat() const
{
    return m_data;
}

cv::Mat& image::mat()
{
    return m_data;
}

void image::show() const
{
    imshow("Display window", m_data);
}

void image::save(const fs::path& path) const
{
    if (!path.empty() && !m_data.empty()) {
        imwrite(path.string(), m_data);
    }
}

int image::operator()(int j, int i) const
{
    return m_data.at<unsigned char>(j,i);
}

int image::operator()(const pixel& p) const
{
    return m_data.at<unsigned char>(p.j,p.i);
}

unsigned char& image::operator()(int j, int i)
{
    return m_data.at<unsigned char>(j,i);
}

unsigned char& image::operator()(const pixel& p)
{
    return m_data.at<unsigned char>(p.j,p.i);
}

bool image::check_color(const pixel& p, Color color) const
{
    return in_range(p) && (*this)(p.j, p.i) == (int)color;
}

bool image::in_range(const pixel& p) const
{
    return (p.j < m_data.rows && p.j >= 0 && p.i < m_data.cols && p.i >= 0);
}

borders image::bfs(const pixel& p, std::unordered_set<pixel, pixel::hash>& visited) const
{
    std::queue<pixel> q;
    q.push(p);
    visited.insert(p);

    // left, right, top, bottm
    borders b{p.i, p.i, p.j, p.j};

    while (!q.empty()) {
        auto current_pixel = q.front();
        q.pop();
        b.update(current_pixel);

        for (int dj : {-1, 0, 1}) {
            for (int di : {-1, 0, 1}) {
                pixel neighbour{current_pixel.j+dj, current_pixel.i+di};
                // if neighbour is black and not yet visited
                if (check_color(neighbour, Color::black) && visited.find(neighbour) == visited.cend()) {
                    q.push(neighbour);
                    visited.insert(neighbour);
                }
            }
        }
    }

    return b;
}

// width, height
std::pair<int,int> image::get_component_avg_size_and_remove_noise()
{
    std::unordered_set<pixel, pixel::hash> visited;
    // width, height
    std::pair<int,int> size_sum{0,0};
    int components_count = 0;

    for (int j = 0; j < rows(); ++j) {
        for (int i = 0; i < cols(); ++i) {
            pixel p{j,i};
            // if pixel is black and not yet visited
            if (check_color(p, Color::black) && visited.find(p) == visited.cend()) {
                int visited_before = visited.size();
                borders b = bfs(p, visited);
                // remove noise
                if (visited.size() - visited_before < 3) {
                    for (int jb = b.top(); jb < b.bottom(); ++jb) {
                        for (int ib = b.left(); ib < b.right(); ++ib) {
                            (*this)(jb, ib) = (int)Color::white;
                        }
                    }
                }
                else {
                    std::pair<int,int> this_s{b.width(), b.height()};
                    if (this_s.first >= 5 && this_s.second >= 5) {
                        size_sum.first += this_s.first;
                        size_sum.second += this_s.second;
                        ++components_count;
                    }
                }
            }
        }
    }

    return {size_sum.first/components_count, size_sum.second/components_count};
}

int image::sum(int begin, int end) const
{
    assert(rows() == 1);
    return std::accumulate(cvit::column_iterator(*this, begin), cvit::column_iterator(*this, end), 0, [](int s, const auto& x) { return s + x(0,0); });
}

void image::fill_row(int row, int value)
{
    auto row_it = cvit::row_iterator(*this, row);
    std::for_each(cvit::column_iterator(*row_it), cvit::column_iterator(), [&](auto&& x) { x(0,0) = value; });
}

void image::fill_row(int row, Color color)
{
    auto row_it = cvit::row_iterator(*this, row);
    std::for_each(cvit::column_iterator(*row_it), cvit::column_iterator(), [&](auto&& x) { x(0,0) = (int)color; });
}

void image::fill(int top, int bottom, Color color)
{
    std::for_each(cvit::row_iterator(*this, top), cvit::row_iterator(*this, bottom), [&](const auto& row) {
        std::for_each(cvit::column_iterator(row), cvit::column_iterator(), [&](auto&& x) { x(0,0) = (int)color; });
    });
}

bool image::row_empty(int row, int start, int end) const
{
    auto row_it = cvit::row_iterator(*this, row);
    return std::none_of(cvit::column_iterator(*row_it, start), cvit::column_iterator(*row_it, end), [](const auto& x) { return x(0,0) == (int)Color::black; });
}

bool image::column_empty(int column, int start, int end) const
{
    auto column_it = cvit::column_iterator(*this, column);
    return std::none_of(cvit::row_iterator(*column_it, start), cvit::row_iterator(*column_it, end), [](const auto& x) { return x(0,0) == (int)Color::black; });
}

void image::threshold(bool otsu)
{
    if (otsu) {
        cv::threshold(m_data, m_data, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    }
    else {
        cv::threshold(m_data, m_data, 200, 255, cv::THRESH_BINARY);
    }
}

image image::operator+(const image& other) const
{
    assert(rows() == other.rows());
    cv::Mat result;
    cv::hconcat(m_data, other.m_data, result);
    return image(std::move(result));
}

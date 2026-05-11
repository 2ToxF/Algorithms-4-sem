#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

static const double kSnapshotAgeSeconds = 60.0;
static const int kOutputPrecision = 20;

class Point {
 public:
  Point() : x_coordinate_(0), y_coordinate_(0) {}

  Point(std::int64_t x_value, std::int64_t y_value)
      : x_coordinate_(x_value), y_coordinate_(y_value) {}

  Point operator+(const Point& other_point) const {
    return Point(x_coordinate_ + other_point.x_coordinate_,
                 y_coordinate_ + other_point.y_coordinate_);
  }

  Point operator-(const Point& other_point) const {
    return Point(x_coordinate_ - other_point.x_coordinate_,
                 y_coordinate_ - other_point.y_coordinate_);
  }

  Point operator-() const { return Point(-x_coordinate_, -y_coordinate_); }

  std::int64_t CrossProduct(const Point& other_point) const {
    return (x_coordinate_ * other_point.y_coordinate_) -
           (y_coordinate_ * other_point.x_coordinate_);
  }

  std::int64_t DotProduct(const Point& other_point) const {
    return (x_coordinate_ * other_point.x_coordinate_) +
           (y_coordinate_ * other_point.y_coordinate_);
  }

  double Length() const {
    std::int64_t square_length = DotProduct(*this);
    double square_length_value = double(square_length);
    return std::sqrt(square_length_value);
  }

  bool IsLowerThan(const Point& other_point) const {
    if (y_coordinate_ < other_point.y_coordinate_) {
      return true;
    }
    if (y_coordinate_ > other_point.y_coordinate_) {
      return false;
    }
    return x_coordinate_ < other_point.x_coordinate_;
  }

  friend std::istream& operator>>(std::istream& input_stream, Point& point) {
    input_stream >> point.x_coordinate_ >> point.y_coordinate_;
    return input_stream;
  }

 private:
  std::int64_t x_coordinate_;
  std::int64_t y_coordinate_;
};

class Polygon {
 public:
  Polygon() = default;

  explicit Polygon(std::size_t polygon_size) : vertices_(polygon_size) {}

  std::size_t Size() const { return vertices_.size(); }

  void Reserve(std::size_t polygon_size) { vertices_.reserve(polygon_size); }

  void PushBack(const Point& point) { vertices_.push_back(point); }

  void PopBack() { vertices_.pop_back(); }

  Point& operator[](std::size_t index) { return vertices_[index]; }

  const Point& operator[](std::size_t index) const { return vertices_[index]; }

  Point GetEdge(std::size_t index) const {
    std::size_t next_index = (index + 1) % Size();
    return vertices_[next_index] - vertices_[index];
  }

  Polygon BuildNegative() const {
    Polygon result_polygon;
    result_polygon.Reserve(Size());

    for (std::size_t index = 0; index < Size(); ++index) {
      result_polygon.PushBack(-vertices_[index]);
    }

    return result_polygon;
  }

  Polygon RotateToLowestVertex() const {
    std::size_t polygon_size = Size();
    std::size_t start_index = FindStartVertex();

    Polygon result_polygon;
    result_polygon.Reserve(polygon_size);

    for (std::size_t index = 0; index < polygon_size; ++index) {
      std::size_t current_index = (start_index + index) % polygon_size;
      result_polygon.PushBack(vertices_[current_index]);
    }

    return result_polygon;
  }

 private:
  std::size_t FindStartVertex() const {
    std::size_t start_index = 0;
    std::size_t polygon_size = Size();

    for (std::size_t index = 1; index < polygon_size; ++index) {
      if (vertices_[index].IsLowerThan(vertices_[start_index])) {
        start_index = index;
      }
    }

    return start_index;
  }

  std::vector<Point> vertices_;
};

enum class EdgeChoice { First, Second, Both };

EdgeChoice ChooseEdge(const Point& first_edge, const Point& second_edge,
                      bool has_first_edge, bool has_second_edge) {
  if (has_first_edge && !has_second_edge) {
    return EdgeChoice::First;
  }
  if (!has_first_edge && has_second_edge) {
    return EdgeChoice::Second;
  }

  std::int64_t cross_value = first_edge.CrossProduct(second_edge);

  if (cross_value > 0) {
    return EdgeChoice::First;
  }
  if (cross_value < 0) {
    return EdgeChoice::Second;
  }
  return EdgeChoice::Both;
}

Point GetNextSumPoint(const Point& current_point, const Point& first_edge,
                      const Point& second_edge, EdgeChoice edge_choice) {
  switch (edge_choice) {
    case EdgeChoice::First:
      return current_point + first_edge;

    case EdgeChoice::Second:
      return current_point + second_edge;

    case EdgeChoice::Both:
      Point sum_edge = first_edge + second_edge;
      return current_point + sum_edge;
  }
}

void MoveEdgeIndices(EdgeChoice edge_choice, std::size_t& first_index,
                     std::size_t& second_index) {
  if (edge_choice == EdgeChoice::First) {
    ++first_index;
    return;
  }
  if (edge_choice == EdgeChoice::Second) {
    ++second_index;
    return;
  }

  ++first_index;
  ++second_index;
}

Polygon BuildMinkowskiSum(const Polygon& first_polygon,
                          const Polygon& second_polygon) {
  Polygon first = first_polygon.RotateToLowestVertex();
  Polygon second = second_polygon.RotateToLowestVertex();

  std::size_t first_size = first.Size();
  std::size_t second_size = second.Size();

  std::size_t first_index = 0;
  std::size_t second_index = 0;

  Point current_point = first[0] + second[0];
  Polygon result_polygon;
  result_polygon.Reserve(first_size + second_size + 1);
  result_polygon.PushBack(current_point);

  while (first_index < first_size || second_index < second_size) {
    Point first_edge(0, 0);
    Point second_edge(0, 0);

    bool has_first_edge = first_index < first_size;
    bool has_second_edge = second_index < second_size;

    if (has_first_edge) {
      first_edge = first.GetEdge(first_index);
    }
    if (has_second_edge) {
      second_edge = second.GetEdge(second_index);
    }

    EdgeChoice edge_choice =
        ChooseEdge(first_edge, second_edge, has_first_edge, has_second_edge);

    current_point =
        GetNextSumPoint(current_point, first_edge, second_edge, edge_choice);
    result_polygon.PushBack(current_point);

    MoveEdgeIndices(edge_choice, first_index, second_index);
  }

  result_polygon.PopBack();
  return result_polygon;
}

double GetSegmentDistanceToOrigin(const Point& segment_start,
                                  const Point& segment_end) {
  Point segment_vector = segment_end - segment_start;
  Point origin_vector = -segment_start;

  std::int64_t projection = origin_vector.DotProduct(segment_vector);
  std::int64_t segment_square = segment_vector.DotProduct(segment_vector);

  if (projection <= 0) {
    return segment_start.Length();
  }
  if (projection >= segment_square) {
    return segment_end.Length();
  }

  std::int64_t area_value = std::abs(segment_start.CrossProduct(segment_end));
  double area = double(area_value);
  double segment_length = segment_vector.Length();

  return area / segment_length;
}

double GetDistanceToBoundary(const Polygon& polygon) {
  double min_distance = std::numeric_limits<double>::max();
  std::size_t polygon_size = polygon.Size();

  for (std::size_t index = 0; index < polygon_size; ++index) {
    std::size_t next_index = (index + 1) % polygon_size;
    double distance =
        GetSegmentDistanceToOrigin(polygon[index], polygon[next_index]);

    min_distance = std::min(distance, min_distance);
  }

  return min_distance;
}

double FindMinimalWaitingTime(const Polygon& airport, const Polygon& cloud) {
  Polygon negative_cloud = cloud.BuildNegative();
  Polygon forbidden_shifts = BuildMinkowskiSum(airport, negative_cloud);

  double total_time = GetDistanceToBoundary(forbidden_shifts);
  double waiting_time = total_time - kSnapshotAgeSeconds;

  if (waiting_time < 0.0) {
    return 0.0;
  }
  return waiting_time;
}

Polygon ReadPolygon(std::size_t polygon_size) {
  Polygon polygon(polygon_size);

  for (std::size_t index = 0; index < polygon_size; ++index) {
    std::cin >> polygon[index];
  }

  return polygon;
}

int main() {
  std::size_t airport_size = 0;
  std::size_t cloud_size = 0;
  std::cin >> airport_size >> cloud_size;

  Polygon airport = ReadPolygon(airport_size);
  Polygon cloud = ReadPolygon(cloud_size);

  double result = FindMinimalWaitingTime(airport, cloud);

  std::cout << std::fixed << std::setprecision(kOutputPrecision);
  std::cout << result << '\n';

  return 0;
}

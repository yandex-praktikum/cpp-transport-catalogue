#include "map_renderer.h"

namespace renderer
{
    namespace detail
    {
        bool LessThenError(double value)
        {
            return std::abs(value) < EPSILON;
        };
    }
    svg::Point SphereProjector::operator()(geo::Coordinates coords) const
    {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_};
    }
}
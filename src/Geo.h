#ifndef Geo_Class
#define Geo_Class

#include <map>
#include <cassert>

/**
 * \class Geo
 * \brief Geo contains geometry info of the LAT
 * \author Xin Chen
 */

class Geo {
 public:
  static const Geo* instance();

  /// get plane number from biLayer and view. Both plane and biLayer starts 
  /// from bottom, view=0(1) refers to measure X(Y).
  int getPlane(int biLayer, int view) const 
    { 
      std::map<BiLayerView, int>::const_iterator itr =
	m_plane.find(BiLayerView(biLayer, view));
      assert(itr != m_plane.end());
      return itr->second; 
    }

  int towerid(int x, int y) const { return y*4 + x; }

 private:
  struct BiLayerView {
    BiLayerView(int biLayer, int view) : m_biLayer(biLayer), m_view(view) { }
    int m_biLayer;
    int m_view;
  };

 public:
  friend bool operator<(const BiLayerView& x, const BiLayerView& y);

 private:
  Geo();
  Geo(const Geo&);

  /// mapping from biLayer and view to plane
  std::map<BiLayerView, int> m_plane;

};

inline const Geo* Geo::instance() 
{
  static Geo geo;
  return &geo;
}

inline bool operator<(const Geo::BiLayerView& x, const Geo::BiLayerView& y)
{
  if(x.m_biLayer != y.m_biLayer) {
    return (x.m_biLayer < y.m_biLayer);
  }
  else if(x.m_view != y.m_view) {
    return (x.m_view < y.m_view);
  }
  else {
    return false;
  }
}

#endif

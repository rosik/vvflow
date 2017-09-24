#include "TBody.hpp"

#include <cstdio>
#include <limits>

TBody::TBody():
    label(),
    alist(),
    speed_x(),
    speed_y(),
    speed_o()
{
    double inf = std::numeric_limits<double>::infinity();
    double nan = std::numeric_limits<double>::quiet_NaN();

    holder = dpos = TVec3D(0., 0., 0.);
    g_dead = 0;
    fdt_dead = TVec3D(0,0,0);
    friction = friction_prev = TVec3D(0,0,0);
    force_hydro = force_holder = TVec3D(0,0,0);
    _slen = _area = 0;
    _cofm = TVec(0., 0.);
    _moi_cofm = 0;
    kspring = TVec3D(inf,inf,inf);
    damping = TVec3D(0., 0., 0.);
    density = 1.;
    special_segment_no = 0;
    boundary_condition = bc_t::steady;
    heat_condition = hc_t::neglect;

    collision_min = collision_max = TVec3D(nan, nan, nan);
    collision_detected = false;
    bounce = 0;
}

TVec3D TBody::speed(double t) const
{
    return TVec3D(
            speed_x.eval(t),
            speed_y.eval(t),
            speed_o.eval(t)
            );
}

void TBody::move(TVec3D deltaHolder, TVec3D deltaBody)
{
    TVec axis = get_axis();
    double _cos = cos(deltaBody.o);
    double _sin = sin(deltaBody.o);
    for (auto& att: alist)
    {
        TVec dr = att.corner - axis;
        att.corner = axis + deltaBody.r + dr*_cos + rotl(dr)*_sin;
    } 
    holder.r += deltaHolder.r;
    holder.o += deltaHolder.o;
    dpos.r += deltaBody.r - deltaHolder.r;
    dpos.o += deltaBody.o - deltaHolder.o;
    speed_slae_prev.r = speed_slae.r;
    speed_slae_prev.o = speed_slae.o;

    doUpdateSegments();
    doFillProperties();
}

void TBody::doUpdateSegments()
{
    if (!alist.size()) {
        return;
    }

    alist.push_back(alist.front());

    for (auto lobj=alist.begin(); lobj<alist.end()-1; lobj++)
    {
        lobj->dl = (lobj+1)->corner - lobj->corner;
        lobj->_1_eps = 3.0/lobj->dl.abs();
        lobj->r = 0.5*((lobj+1)->corner + lobj->corner);
    }
    alist.pop_back();
}

TAtt* TBody::isPointInvalid(TVec p)
{
    return isPointInContour(p, alist);
}

TAtt* TBody::isPointInHeatLayer(TVec p)
{
    if (!heat_layer.size())
    {
        for (auto& lobj: alist)
        {
            heat_layer.push_back(lobj.r + rotl(lobj.dl));
        }
    }

    return isPointInContour(p, heat_layer);
}

template <class T> TVec corner(T lobj);
template <> inline TVec corner(TVec lobj) {return lobj;}
template <> inline TVec corner(TAtt lobj) {return lobj.corner;}
template <class T>
TAtt* TBody::isPointInContour(TVec p, std::vector<T> &list)
{
    bool inContour = isInsideValid();
    if ( !inContour && (
        p.x < _min_rect_bl.x ||
        p.y < _min_rect_bl.y ||
        p.x > _min_rect_tr.x ||
        p.y > _min_rect_tr.y ||
        (p-_cofm).abs2() > _min_disc_r2
        )) return NULL;

    TAtt *nearest = NULL;
    double nearest_dr2 = std::numeric_limits<double>::max();

    for (auto i = list.begin(), j = list.end()-1; i<list.end(); j=i++)
    {
        TVec vi = corner<T>(*i);
        TVec vj = corner<T>(*j);

        if ((
                    (vi.y < vj.y) && (vi.y < p.y) && (p.y <= vj.y) &&
                    ((vj.y - vi.y) * (p.x - vi.x) > (vj.x - vi.x) * (p.y - vi.y))
            ) || (
                (vi.y > vj.y) && (vi.y > p.y) && (p.y >= vj.y) &&
                ((vj.y - vi.y) * (p.x - vi.x) < (vj.x - vi.x) * (p.y - vi.y))
                )) inContour = !inContour;
    }

    if (!inContour) return NULL;

    // FIXME почитать алгоритмы, поискать оптимизированный поиск минимума
    for (auto& latt: alist)
    {
        double dr2 = (latt.r - p).abs2();
        if ( dr2 < nearest_dr2 )
        {
            nearest = &latt;
            nearest_dr2 = dr2;
        }
    }

    return nearest;
}

void TBody::doFillProperties()
{
    _slen = 0;
    _area = 0;
    TVec _3S_cofm = TVec(0., 0.);
    double _12moi_0 = 0.; // 12 * moment of inertia around point (0, 0)

    if (!alist.size()) {
        return;
    }

    alist.push_back(alist.front());
    for (auto latt=alist.begin(); latt<alist.end()-1; latt++)
    {
        _slen+= latt->dl.abs();
        _area+= latt->r.y*latt->dl.x;
        _3S_cofm-= latt->r * (rotl(latt->corner) * (latt+1)->corner);
        _12moi_0 -= (latt->corner.abs2() + latt->corner*(latt+1)->corner + (latt+1)->corner.abs2())
            *  (rotl(latt->corner) * (latt+1)->corner);

    }
    alist.pop_back();
    _cofm = _3S_cofm/(3*_area);
    _moi_cofm = _12moi_0/12. - _area*_cofm.abs2();

    _min_disc_r2 = 0;
    _min_rect_bl = TVec(
            std::numeric_limits<double>::infinity(),
            std::numeric_limits<double>::infinity());
    _min_rect_tr = TVec(
            -std::numeric_limits<double>::infinity(),
            -std::numeric_limits<double>::infinity());
    for (auto& latt: alist)
    {
        double r2 = (latt.corner-_cofm).abs2();
        if (r2 > _min_disc_r2) _min_disc_r2 = r2;
        if (latt.corner.x > _min_rect_tr.x) _min_rect_tr.x = latt.corner.x;
        if (latt.corner.y > _min_rect_tr.y) _min_rect_tr.y = latt.corner.y;
        if (latt.corner.x < _min_rect_bl.x) _min_rect_bl.x = latt.corner.x;
        if (latt.corner.y < _min_rect_bl.y) _min_rect_bl.y = latt.corner.y;
    }
}

inline double atan2(const TVec &p)
{
    return atan2(p.y, p.x);
}

/********************************** SAVE/LOAD *********************************/
int TBody::load_txt(const char* filename)
{
    alist.clear();

    FILE *fin = fopen(filename, "r");
    if (!fin) { return errno; }

    TAtt att;
    int  err = 0;
    char line[128];
    while ( !err && !feof(fin) && !ferror(fin) && fgets(line, sizeof(line), fin) )
    {
        err |= sscanf(line, "%lf %lf %u", &att.corner.x, &att.corner.y, &att.slip) < 2;
        alist.push_back(att);
    }

    err |= ferror(fin);
    fclose(fin);

    if (err) {
        return errno?:EINVAL;
    }

    doUpdateSegments();
    doFillProperties();

    return 0;
}

#include "morefilter.h"

Tofbf::Tofbf(int speed) { curr_speed_ = speed; }

Tofbf::~Tofbf() {}

/**
 * @brief Filter within 5m to filter out unreasonable data points
 * @param [in]
 * 	@param tmp lidar point data
 * @return std::vector<PointData>
 */
std::vector<PointData> Tofbf::NearFilter(
    const std::vector<PointData> &tmp) const
{
    std::vector<PointData> normal, pending, item;
    std::vector<std::vector<PointData>> group;

    // Remove points within 5m
    for (auto n : tmp)
    {
        if (n.distance < 5000)
        {
            pending.push_back(n);
        }
        else
        {
            normal.push_back(n);
        }
    }

    if (tmp.empty())
        return normal;

    double angle_delta_up_limit = curr_speed_ / kScanFrequency * 2;//kScanFrequency -> 默认扫描频率

    // std::cout <<angle_delta_up_limit << std::endl; test code

    // sort
    std::sort(pending.begin(), pending.end(),
              [](PointData a, PointData b)
              { return a.angle < b.angle; });

    PointData last(-10, 0, 0, 0);
    // group
    for (auto n : pending)
    {
        if (abs(n.angle - last.angle) > angle_delta_up_limit ||
            abs(n.distance - last.distance) > last.distance * 0.03)
        {
            if (item.empty() == false)
            {
                group.push_back(item);
                item.clear();
            }
        }
        item.push_back(n);
        last = n;
    }
    // push back last item
    if (item.empty() == false)
        group.push_back(item);

    if (group.empty())
        return normal;

    // Connection 0 degree and 359 degree
    auto first_item = group.front().front();
    auto last_item = group.back().back();
    if (abs(first_item.angle + 360.f - last_item.angle) < angle_delta_up_limit &&
        abs(first_item.distance - last_item.distance) < last.distance * 0.03)
    {
        group.front().insert(group.front().begin(), group.back().begin(),
                             group.back().end());
        group.erase(group.end() - 1);
    }
    // selection
    for (auto n : group)
    {
        if (n.size() == 0)
            continue;
        // No filtering if there are many points
        if (n.size() > 15)
        {
            normal.insert(normal.end(), n.begin(), n.end());
            continue;
        }

        // Filter out those with few points
        if (n.size() < 3)
        {
            int c = 0;
            for (auto m : n)
            {
                c += m.confidence;
            }
            c /= n.size();
            if (c < kConfidenceSingle)
                continue;
        }

        // Calculate the mean value of distance and confidence
        double confidence_avg = 0;
        double dis_avg = 0;
        for (auto m : n)
        {
            confidence_avg += m.confidence;
            dis_avg += m.distance;
        }
        confidence_avg /= n.size();
        dis_avg /= n.size();

        // High confidence, no filtering
        if (confidence_avg > kConfidenceLow)
        {
            normal.insert(normal.end(), n.begin(), n.end());
            continue;
        }
    }
    std::sort(normal.begin(), normal.end(), [](PointData a, PointData b)
              { return a.timeStamp < b.timeStamp; });

    return normal;
}
std::vector<PointData> Tofbf::NoiseFilter(const std::vector<PointData> &tmp) const
{
    std::vector<PointData> normal;
    PointData last_data, next_data;

    if (tmp.empty())
        return normal;

    // Traversing the point data
    int count = 0;
    for (auto n : tmp)
    {
        if (count == 0)
        {
            last_data = tmp[tmp.size() - 1];
        }
        else
        {
            last_data = tmp[count - 1];
        }

        if (count == (int)(tmp.size() - 1))
        {
            next_data = tmp[0];
        }
        else
        {
            next_data = tmp[count + 1];
        }
        count++;

        // Remove points with the opposite trend within 500mm
        if (n.distance < 500)
        {
            if ((n.distance + 10 < last_data.distance && n.distance + 10 < next_data.distance) ||
                (n.distance > last_data.distance + 10 && n.distance > next_data.distance + 10))
            {
                if (n.confidence < 40)
                {
                    n.confidence = 0;
                    n.distance = 0;
                    normal.push_back(n);
                    continue;
                }
            }
            else if ((n.distance + 7 < last_data.distance && n.distance + 7 < next_data.distance) ||
                     (n.distance > last_data.distance + 7 && n.distance > next_data.distance + 7))
            {
                if (n.confidence < 30)
                {

                    n.confidence = 0;
                    n.distance = 0;
                    normal.push_back(n);
                    continue;
                }
            }
            else if ((n.distance + 5 < last_data.distance && n.distance + 5 < next_data.distance) ||
                     (n.distance > last_data.distance + 5 && n.distance > next_data.distance + 5))
            {
                if (n.confidence < 20)
                {
                    n.confidence = 0;
                    n.distance = 0;
                    normal.push_back(n);
                    continue;
                }
            }
        }

        // Remove points with very low intensity within 5m
        if (n.distance < 5000)
        {
            /* if (n.distance < 200)
      {
        if (n.confidence < 25)
        {
          n.confidence = 0;
          n.distance = 0;
          normal.push_back(n);
          continue;
        }
      }
      else
      {
        if (n.confidence < 10)
        {
          n.confidence = 0;
          n.distance = 0;
          normal.push_back(n);
          continue;
        }
      } */

            if ((n.distance + 30 < last_data.distance || n.distance > last_data.distance + 30) &&
                (n.distance + 30 < next_data.distance || n.distance > next_data.distance + 30))
            {
                if ((n.distance < 2000 && n.confidence < 30) || n.confidence < 2)
                {
                    n.confidence = 0;
                    n.distance = 0;
                    normal.push_back(n);
                    continue;
                }
            }
        }
        normal.push_back(n);
    }
    return normal;
}
std::vector<PointData> Tofbf::ShadowsFilter(const std::vector<PointData> &scan_in) const
{
    std::vector<PointData> data(scan_in);
    std::set<int> indices_to_delete;
    int param_window = 5; //20
    double param_min_angle = 15, param_max_angle = 165;

    double angle_increment = M_PI * 2 / 720; //600
    int index_l = (int)(data.size() - param_window - 1);

    if(index_l < 0) return data;

    for (int i = 0; i < index_l; i++)
    {
        if (data[i].distance < 2)
            continue;

        for (int y = 1; y < param_window + 1; y++)
        {
            int j = i + y;
            if (data[j].distance < 2)
                continue;
            if (j < 0 || j >= (int)data.size() || (int)i == j)
                continue;
            if (fabs(data[i].distance - data[j].distance) < 200)
                continue;

            double rad = atan2(data[j].distance * sin(y * angle_increment),
                               data[i].distance - data[j].distance * cos(y * angle_increment));
            double angle = fabs(rad * 180 / M_PI);
            if (angle < param_min_angle || angle > param_max_angle)
            {
                int from, to;
                if (data[i].distance < data[j].distance)
                {
                    from = i + 1;
                    to = j;
                }
                else {
                    from = j - 1;
                    to = i;
                }
                if (from > to) {
                    int t = from;
                    from = to;
                    to = t;
                }
                for (int index = from; index <= to; index++)
                    indices_to_delete.insert(index);
            }
        }
    }
    for (std::set<int>::iterator it = indices_to_delete.begin(); it != indices_to_delete.end(); ++it)
        data[*it].distance = 0;

    return data;
}
std::vector<PointData> Tofbf::MedianFilter(const std::vector<PointData> &scan_in) const
{
    std::vector<PointData> out_data(scan_in);

    int param_window = 5;
    int dists[720];
    int buf[param_window * 2 + 1];

    int index_l = (int)(out_data.size() - param_window - 1);

    if(out_data.empty() || index_l < 0)
        return out_data;

    for (int i = 0; i < out_data.size(); i++)
        dists[i] = out_data[i].distance;

    for (int i = param_window; i < index_l; i++)
    {
        if (dists[i] == 0) continue;
        int n = 0;
        for (int j = -param_window; j <= param_window; j++)
        {
            if (dists[i + j] > 0) {
                buf[n++] = dists[i + j];
            }
        }
        if (n > 2)
        {
            std::sort(buf, buf+n);
            out_data[i].distance = buf[param_window];
        }
    }

    return out_data;
}
std::vector<PointData> Tofbf::WanderFilter(const std::vector<PointData> &scan_in) const
{
    std::vector<PointData> out_data;
    uint16_t check_t = 5;
    uint16_t ranges_t = 11;
    float angle_t = 1.5;

    if(scan_in.size() <= check_t)
        return scan_in;

    for(ssize_t i = 0; i < scan_in.size(); i++)
    {
        auto scan = scan_in[i];
        size_t casual_cunt = 0;
        float angle_last ,angle_next;
        uint16_t ranges_last;
        uint16_t ranges = scan_in[i].distance;

        if(ranges < kdisMin || ranges > kdisMax)
        {
            out_data.push_back(scan);
            continue;
        }

        for(ssize_t j = i - 1; j >= 0; j--)
        {
            angle_last = scan_in[j].angle;
            angle_next = scan_in[j+1].angle;
            ranges_last = scan_in[j].distance;
            if((abs(ranges - ranges_last) < ranges_t) && (fabs(angle_next - angle_last) < angle_t))
            {
                casual_cunt++;
                continue;
            }
            else
            {
                break;
            }
        }
        for(ssize_t j = i + 1; j < scan_in.size(); j++)
        {
            angle_last = scan_in[j].angle;
            angle_next = scan_in[j-1].angle;
            ranges_last = scan_in[j].distance;
            if((fabs(ranges  - ranges_last) < ranges_t) && (fabs(angle_next - angle_last) < angle_t))
            {
                casual_cunt++;
                continue;
            }
            else
            {
                break;
            }
        }

        if(casual_cunt > check_t)
            out_data.push_back(scan);
    }

    return out_data;
}

std::vector<PointData> Tofbf::TineFilter(const std::vector<PointData> &scan_in) const
{
    std::vector<PointData> out_data(scan_in);
    std::vector<PointData> pending;
    size_t param_window = 20;

    auto scansize = scan_in.size();
    if(scansize <= param_window)
        return scan_in;

    size_t i;
    for(i = param_window/2; i < scansize - param_window/2;)
    {
        auto scan = scan_in[i];
        auto scan_l = scan_in[i];
        auto scan_r = scan_in[i];
        size_t l_count = 0, r_count = 0, overdis_count = 0;
        if(scan.distance < kdisMin || scan.distance > kdisMax)
        {
            i++;
            continue;
        }

        overdis_count = 0;
        for(ssize_t k_l = i - 1; k_l > i - param_window/2; k_l--)
        {
            auto pend = scan_in[k_l];

            if(pend.distance < kdisMin || pend.distance > kdisMax)
                overdis_count ++;
            if(overdis_count > 2)
                break;

            if(scan_l.distance < pend.distance && abs(scan_r.distance - pend.distance) > 2)
            {
                scan_l = pend;
                l_count ++;
            }
            else if(scan_l.distance > pend.distance + 5)
            {
                break;
            }
        }

        overdis_count = 0;
        for(ssize_t k_r = i + 1; k_r < i + param_window/2; k_r++)
        {
            auto pend = scan_in[k_r];

            if(pend.distance < kdisMin || pend.distance > kdisMax)
                overdis_count ++;
            if(overdis_count > 2)
                break;

            if(scan_r.distance < pend.distance && abs(scan_r.distance - pend.distance) > 2)
            {
                scan_r = pend;
                r_count ++;
            }
            else if(scan_r.distance > pend.distance + 5)
            {
                break;
            }
        }

        if(l_count == 0 || r_count == 0)
        {
            i += param_window/2;
            continue;
        }
        else if((l_count >= 5) && (r_count >= 5))
        {
            float rad_average = (scan_l.angle + scan_r.angle)/2;
            uint16_t dis_average = (scan_l.distance + scan_r.distance)/2;
            uint16_t dis_scan = (uint16_t)(dis_average*cos((rad_average - scan_l.angle)*M_PI/180));

            for(size_t j = i - param_window/2; j <= i + param_window/2; j++)
            {
                float RadCos = (float)cos((fabs(scan.angle - out_data[j].angle)*M_PI/180));

                if(RadCos != 0)
                    out_data[j].distance = (uint16_t)(dis_scan/RadCos);
            }

            i += param_window/2;
            continue;
        }

        i++;
    }

    return out_data;
}

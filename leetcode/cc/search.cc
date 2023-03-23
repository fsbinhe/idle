#include <vector>

// [l, r)
int binary_search_v1(std::vector<int> &nums, int target)
{
  int l = 0;
  int r = nums.size();

  while (l < r)
  {
    int mid = l + (r - l) / 2;
    if (nums[mid] == target)
    {
      return mid;
    }
    else if (nums[mid] < target)
    {
      l = mid + 1;
    }
    else
    {
      r = mid;
    }
  }

  return -1;
}

// [l..r]
int binary_search_v2(std::vector<int> &nums, int target)
{
  int l = 0;
  int r = nums.size() - 1;

  while (l <= r)
  {
    int mid = l + (r - l) / 2;
    if (nums[mid] == target)
    {
      return mid;
    }
    else if (nums[mid] < target)
    {
      l = mid - 1;
    }
    else
    {
      r = mid + 1;
    }
  }

  return -1;
}
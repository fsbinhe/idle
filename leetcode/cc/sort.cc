#include <vector>
#include <algorithm>

// Insertion Sort:
// Input: nums
// Output: sorted nums
void insertSort(std::vector<int> &nums)
{
  int n = nums.size();
  if (n == 0 || n == 1)
  {
    return;
  }

  for (int j = 1; j < n; j++)
  {
    int key = nums[j];
    int i = j - 1;
    while (i >= 0 && nums[i] >= key)
    {
      nums[i + 1] = nums[i];
      i--;
    }
    nums[i + 1] = key;
  }
}

// mergeSort for nums in range [l..r]
void mergeSort(std::vector<int> &nums, int l, int r)
{
  if (l >= r)
  {
    return;
  }
  int mid = (l + r) / 2;
  mergeSort(nums, l, mid);
  mergeSort(nums, mid + 1, r);

  std::vector<int> aux;
  int i = l;
  int j = mid + 1;
  while (i <= mid && j <= r)
  {
    if (nums[i] <= nums[j])
    {
      aux.push_back(nums[i++]);
    }
    else
    {
      aux.push_back(nums[j++]);
    }
  }

  while (i <= mid)
    aux.push_back(nums[i++]);
  while (j <= r)
    aux.push_back(nums[j++]);

  int k = l;
  for (int i = 0; i < aux.size(); i++)
  {
    nums[k++] = aux[i];
  }
}

void countSort(std::vector<int> &nums, int maxValue)
{
  std::vector<int> count(maxValue);
  for (int i = 0; i < nums.size(); i++)
  {
    count[nums[i]]++;
  }

  for (int i = 0; i < count.size(); i++)
  {
    count[i] += count[i - 1];
  }

  std::vector<int> aux(nums.size());

  for (int i = nums.size() - 1; i >= 0; i--)
  {
    aux[--count[nums[i]]] = nums[i];
  }

  for (int i = 0; i < nums.size(); i++)
  {
    nums[i] = aux[i];
  }
}

// partition nums in range [l..r]
// return i
// [l..i-1] <= nums[i] <= nums[i+1..r]
int partition(std::vector<int> &nums, int l, int r)
{
  int x = nums[r];
  int i = l - 1;
  for (int j = l; j <= r; j++)
  {
    if (nums[j] <= x)
    {
      std::swap(nums[++i], nums[j]);
    }
  }

  return i;
}

void quickSort(std::vector<int> &nums, int l, int r)
{
  if (l >= r)
  {
    return;
  }

  int p = partition(nums, l, r);
  quickSort(nums, l, p - 1);
  quickSort(nums, p + 1, r);
}

void radixSort(std::vector<int> &nums)
{
  int maxValue = *std::max_element(nums.begin(), nums.end());
  int n = nums.size();
  int exp = 1;
  int radix = 10;

  std::vector<int> aux(n);

  while (maxValue / exp > 0)
  {
    std::vector<int> count(radix, 0);
    for (int i = 0; i < n; i++)
    {
      count[(nums[i] / exp) % radix]++;
    }
    for (int i = 1; i < count.size(); i++)
    {
      count[i] += count[i - 1];
    }
    for (int i = n - 1; i >= 0; i--)
    {
      aux[--count[(nums[i] / exp) % radix]] = nums[i];
    }
    exp *= 10;
  }
  for (int i = 0; i < n; i++)
  {
    nums[i] = aux[i];
  }
}
#include <bits/stdc++.h>

#define rep(i, N) for (int i = 0; i < N; i++)
#define rep2(i, N) for (int i = 1; i <= N; i++)
using namespace std;
long long INF = 1e18;
long long mod = 1e9 + 7;

int dx[4] = {1, 0, -1, 0};
int dy[4] = {0, 1, 0, -1};

#define debug 0
#define debugWorkTime 0
#define debugWorkTimeMax 1
#define debugWorkTimeMin 0
#define debugWorkTimeAverage 0
#define sortByMaxMinAve 1

#define debugWorkMember 0
#define useSample 0
#define useTestDate 0
#define setRandonSelect 0

vector<vector<int>> from(1010);
vector<vector<int>> to(1010);

// taskSkill[i][k]:=タスクiに必要な技能k
double taskSkill[1010][25];

// timeToNeed[i][k]:=メンバーiがタスクkを終了するまでにかかる日数。
// long long timeToNeed[25][1010];

// assing[i]:=メンバーiに割り振られたタスク、タスクがなければ-1を入れる
vector<long long> taskAssign(25);

// assing[i]:=タスクiに割り振られたメンバー、メンバがなければ-1を入れる
vector<long long> memberAssign(25);

//メンバー　iがやっている仕事にかかる時間
vector<long long> timeUntilFinish(25, INF);

// finished[i]:=タスクiが終了したかどうか.終了していたらtrue
bool finished[1010] = {};

// 進行中かどうか、進行中ならtrue
bool onGoing[1010] = {};

// vector<pair<前のタスクにかかった時間,メンバーの番号>>selectOrder
vector<pair<double, int>> selectOrder(25);

//現在のタスクにかかっている時間
vector<int> workingTime(25, 0);

// completedTask[i]:=メンバーiがタスクをクリアした数
vector<int> completedTask(25, 0);

//タスク同士の類似度
double similarity[1010][1010] = {};

//タスクiが達成されるのにかかった日数と
//その仕事をやった人を記録
vector<pair<int, double>> completeTimeAndMember(1010, make_pair(-1, 1e5));

struct MemberInfo {
  //タスクに要した時間の最大,最小、平均
  double WorkTimeMax;
  double WorkTimeMin;
  double WorkTimeAve;
  double nowWorkTime;

  //コンストラクタ
  MemberInfo() {
    WorkTimeMin = 1e5;
    WorkTimeMax = 0.0;
    WorkTimeAve = 0.0;
    nowWorkTime = -1.0;
  }
  //タスクをアップデート
  void updateInfo() {}

  double getRateValue() {
    return WorkTimeAve * 0.1 + WorkTimeMax * 0.3 + WorkTimeMin * 0.3;
  }
};

//メンバーを格納する配列
vector<MemberInfo> Members(25);

//ソートの比較関数
bool sortMembers(MemberInfo m1, MemberInfo m2) { return true; }

//ソートの比較関数 小さい順にソート
bool cmp(pair<double, int> p1, pair<double, int> p2) {
  return p1.first < p2.first;
}
//降順にソートする
bool sortByDown(pair<double, int> p1, pair<double, int> p2) {
  return p1.first < p2.first;
}

void makeSelectOrder(int m) {
  rep(i, m) selectOrder[i] = make_pair(0.5, i);
  return;
}

//タスクxとタスクyの類似度を求める
// 類似度=cos類似度とする
double getSim(int x, int y, int k) {
  double xSumSq = 0.0, ySumSq = 0.0, xySum = 0.0;

  rep(i, k) {
    xSumSq += taskSkill[x][i];
    ySumSq += taskSkill[y][i];
    xySum += taskSkill[x][i] * taskSkill[y][i];
  }
  xSumSq = sqrt(xSumSq);
  ySumSq = sqrt(ySumSq);
  return xySum / (xSumSq * ySumSq);
}

//全てのタスク同士の類似度を求める
void calSimilarity(int n, int k) {
  for (int i = 0; i < n; i++) {
    for (int l = i + 1; l < n; l++) {
      similarity[i][l] = similarity[l][i] = getSim(i, l, k);
    }
  }
}
//タスクiに関する類似度をソートした配列を返す (タスク番号i,タスク数n)
vector<pair<double, int>> getSimVec(int i, int n) {
  vector<pair<double, int>> ret;
  rep(l, n) {
    if (l != i) ret.push_back(make_pair(similarity[i][l], l));
  }
  sort(ret.begin(), ret.end(), sortByDown);
  return ret;
}

//依存関係の入力と構築
void makeDependency(int R) {
  //依存関係
  rep(i, R) {
    int u, v;
    cin >> u >> v;
    u--, v--;
    //依存関係をtoとfromで表現
    // to[1]=2　の時1->２の依存関係がある
    to[u].push_back(v);
    // from[2]=1の時2->の依存関係がある
    from[v].push_back(u);
  }
}
void makeTasks(int N, int K) {
  // taskに必要な技能
  rep(i, N) {
    rep(l, K) { cin >> taskSkill[i][l]; }
  }
}

//ランダムな0〜nまでの数列を返す
vector<int> getRandomVec(int n) {
  vector<int> shuffle(n);
  rep(i, n) shuffle[i] = i;
  std::random_device seed_gen;
  std::mt19937 engine(seed_gen());
  std::shuffle(shuffle.begin(), shuffle.end(), engine);
  return shuffle;
}

//  タスクkが現在取り組むことができるか判定
//  取り組めるならtrueを返す
bool taskIsReady(int k) {
  //すでに進行中又は終了後だったらダメ
  if (onGoing[k] || finished[k]) return false;

  //依存するタスクが全て終わっているかチェック
  for (auto nx : from[k]) {
    if (!finished[nx]) return false;
  }
  return true;
}

//  メンバーiの仕事が終わったことを記録
void setFinish(int i) {
  //担当していたタスクの終了を記録
  finished[taskAssign[i]] = true;
  //進行中フラグを閉じる
  onGoing[taskAssign[i]] = false;
  //タスクにかかった時間を記録する
  completeTimeAndMember[taskAssign[i]] = make_pair(i, (double)workingTime[i]);
  // printf("#task%d was done by member%d in
  // %3.0lfdays\n",taskAssign[i],i,completeTimeAndMember[i].second);

  //かかった時間を記録
  rep(k, 25) {
    if (selectOrder[k].second == i) {
      //今までの作業にかかった時間の最大値で更新
      // selectOrder[k].first = max(selectOrder[k].first, workingTime[i]);

      //今までの作業にかかった時間の最小値で更新
      if (selectOrder[k].first == 0.0)
        selectOrder[k].first = (double)workingTime[i];
      else {
#if debugWorkTimeMin

        selectOrder[k].first =
            min(selectOrder[k].first, (double)workingTime[i]);
#elif 0  // debugWorkTimeMax

        selectOrder[k].first =
            max(selectOrder[k].first, (double)workingTime[i]);
#elif sortByMaxMinAve
        //最小値
        Members[i].WorkTimeMin =
            min(Members[i].WorkTimeMin, (double)workingTime[i]);
        //平均値
        Members[i].WorkTimeAve =
            (double)(Members[i].WorkTimeAve * completedTask[i] +
                     workingTime[i]) /
            (completedTask[i] + 1.0);
        //最大値
        Members[i].WorkTimeMax =
            max(Members[i].WorkTimeMax, (double)workingTime[i]);
        selectOrder[k].first = Members[i].getRateValue();
#if 0
        // debug用
        printf("# member %d (Max %lf Min %lf Ave %lf Rate =%lfM )\n",i, Members[i].WorkTimeMax,
               Members[i].WorkTimeMin, Members[i].WorkTimeAve,
               Members[i].getRateValue());
#endif
#endif
      }
      break;
    }
  }
  //現在こなしたタスク数を増やす
  completedTask[i]++;
  //現在の作業時間を0にする
  workingTime[i] = 0;
  //メンバーiに割り当てられたタスクをからにする
  taskAssign[i] = -1;
}

//空いているタスクがあれば取ってくる、なければfalse
// メンバーi タスク k
bool getTask(int i, int n) {
  rep(k, n) {
    //タスクkが開始可能であれば割り当てる
    if (taskIsReady(k)) {
      taskAssign[i] = k;
      memberAssign[k] = i;
      onGoing[k] = true;
      // timeUntilFinish[i] = timeToNeed[i][k];
      return true;
    }
  }
  return false;
}
//空いているメンバーがいれば割り当てる
//　タスクi メンバー数m
bool getMember(int i, int m) {
  rep(k, m) {
    //メンバーkが労働可能であれば割り当てる
    if (taskAssign[k] < 0) {
      taskAssign[k] = i;
      memberAssign[i] = k;
      onGoing[i] = true;
      return true;
    }
  }
  return false;
}

void updateProgress(int M) {
  rep(i, M) {
    if (taskAssign[i] > -1) {
      //残り日数を1日減らす
      timeUntilFinish[i]--;
      //
      if (timeUntilFinish[i] <= 0) setFinish(i);
    }
  }
}

//メンバーの番号が若い順にタスクを割り当てていく
void simpleAssign(int m, int n) {
  //今日作業を開始する人とタスクのペアが入った出力結果
  vector<pair<int, int>> output;

  vector<int> shuffle(m);
  //最初の割り当てをランダムに行うとき
#if setRandonSelect
  shuffle = getRandomVec(notWorkedMembers);
#else
  rep(i, m) shuffle[i] = i;
#endif

  rep(l, m) {
    int i = shuffle[l];
    if ((taskAssign[i] < 0) && getTask(i, n)) {
      output.push_back(make_pair(i, taskAssign[i]));
    }
  }
  //出力用
  printf("%d ", output.size());
  for (auto now : output) {
    printf("%d %d ", now.first + 1, now.second + 1);
  }
  printf("\n");
}

//前回のタスクの完了が早い人から優先して採用
void assignByFastMember(int m, int n) {
  vector<pair<int, int>> output;

  //コストが軽い順にソート
  sort(selectOrder.begin(), selectOrder.begin() + m, cmp);

  //何番目のメンバーまでまだタスクをしたことがないか確認
  int notWorkedMembers = 0;
  rep(i, m) {
    if (selectOrder[i].first > 0.0) {
      notWorkedMembers = i;
      break;
    }
  }
#if debugWorkMember
  printf("#s 現在 %d人がまだ働いていない\n", notWorkedMembers);
#endif
  vector<int> shuffle(notWorkedMembers);
#if setRandonSelect
  shuffle = getRandomVec(notWorkedMembers);
#else
  rep(i, notWorkedMembers) shuffle[i] = i;
#endif
  //まだ情報がない部分はシャッフルしてランダムに選ぶ
  rep(l, m) {
    int i = l;
    if (l < notWorkedMembers) i = shuffle[l];

    int number = selectOrder[i].second;
    if ((taskAssign[number] < 0) && getTask(number, n)) {
      output.push_back(make_pair(number, taskAssign[number]));
    }
  }

  //出力用
  printf("%d ", output.size());
  for (auto now : output) {
    printf("%d %d ", now.first + 1, now.second + 1);
  }
  printf("\n");
}

//タスクiについて、類似度が高いタスクを早くこなした
//メンバーのindexを返す
int getOptMember(int i, int n) {
  //メンバーiの他のタスクとの類似度が<double,int>のpairで入ったベクトル
  //降順にソートずみ
  vector<pair<double, int>> vec = getSimVec(i, n);
  double threshold = 10.0;
  rep(l, n - 1) {
    // now<類似度,タスク番号>
    pair<double, int> now = vec[l];
    if (now.second < 0) continue;
    //一定時間よりも短い時間で終わらせていて
    //かつその人が今何もしていなければ割り当てる
    if (completeTimeAndMember[now.second].second < threshold &&
        (taskAssign[completeTimeAndMember[now.second].first] < 0)) {
      return completeTimeAndMember[now.second].first;
    }
  }
  return -1;
}

//現在取り組めるタスクについて
//似たタスクを早く終わらせた人に割り当てる
void assignByTaskSimilarity(int m, int n) {
  vector<pair<int, int>> output;  //出力用ベクトル

  //取り組めるタスクについて、割り当てる
  // rep(task, n) {
  //   int findGoodMember = getOptMember(task, n);
  //   //いいメンバーが見つかなかったら-1が帰ってくる
  //   if (taskIsReady(task) && findGoodMember >= 0) {
  //     // printf("# assigned 1\n");
  //     taskAssign[findGoodMember] = task;
  //     onGoing[task] = true;
  //     output.push_back(make_pair(findGoodMember, task));
  //   }
  // }
  rep(task, n) {
    if(!taskIsReady(task))continue;

    int findGoodMember = getOptMember(task, n);
    //いいメンバーが見つかなかったら-1が帰ってくる
    if (findGoodMember >= 0) {
      //printf("# assigned 1\n");
      taskAssign[findGoodMember] = task;
      memberAssign[task] = findGoodMember;
      onGoing[task] = true;
      output.push_back(make_pair(findGoodMember, task));
    } else if (taskIsReady(task)) {
      //作業可能なメンバーがいれば割り当てる
      if (getMember(task, m)) {
        //printf("# assigned\n");
        output.push_back(make_pair(memberAssign[task], task));
      }
    }
  }
  printf("%d ", output.size());
  for (auto now : output) {
    printf("%d %d ", now.first + 1, now.second + 1);
  }
  printf("\n");
  return;
}

//メンバーiが今日働いた分を記録
void registWorkTime(int m) {
  rep(i, m) {
    if (taskAssign[i] != -1) {
      workingTime[i]++;
    }
  }
  return;
}

//デバッグ用　全員のタスク割り当てを出力
void printNowAssignMent(int m) {
  rep(i, m) {
    int taskStatus = taskAssign[i];
    if (taskStatus == -1) {
      printf("メンバー %d の　進行中のタスクはありません\n", i + 1);
    } else {
      printf("メンバー %d の　進行中のタスクは　タスク%d です\n", i + 1,
             taskStatus + 1);
    }
  }
}

//全てのタスクが終わっているか確認する
bool allTasksCompleted(int n) {
  rep(i, n) {
    if (!finished[i]) return false;
  }
  return true;
}

//現在のタスクの

int main() {
  long long N, M, K, R;
  cin >> N >> M >> K >> R;

  //タスク関係の入力
  makeTasks(N, K);
  //依存関係の入力と構築
  makeDependency(R);
  //メンバーを整列させて初期化
  makeSelectOrder(M);

  calSimilarity(N, K);
#if 0
  for (int i = 0; i < N; i++) {
    for (int l = i + 1; l < N; l++) {
      if(similarity[i][l]>9.0)printf("# similarity %d %d = %lf\n", i, l, similarity[i][l]);
    }
  }
#endif
  int status = -1;
  rep(i, M) taskAssign[i] = -1;
#if useTestDate
  rep(i, M) {
    rep(k, K) {
      int skill;
      cin >> skill;
    }
  }
  rep(i, N) {
    rep(l, M) {
      int skill;
      cin >> timeToNeed[l][i];
    }
  }
#endif
  //初日の割り当て
  simpleAssign(M, N);
  int days = 0;
  while (1) {
    days++;
    //メンバーの労働時間を記録
    registWorkTime(M);
#if useSample
    //今日の分の進み具合を更新
    updateProgress(M);
    if (allTasksCompleted(N) || days > 2000) break;
#else
    //今日仕事を終えたメンバーを記録
    cin >> status;
    if (status < 0) break;
    rep(i, status) {
      int finishedMember;
      cin >> finishedMember;
      finishedMember--;
      setFinish(finishedMember);
    }
#endif
#if debug
    printf("0 now status =%d\n", status);
#endif
    // printf("#s today %d\n", days);
    // printf("#s score = %d+%d-%d = %d\n", N, 2000, days, N + 2000 - days);

    //メンバーの仕事が早い順に割り当てていく方法
    // assignByFastMember(M, N);

    //何も考えずに番号が若い順にタスクを割り当てる方法
    // simpleAssign(M, N);

    //タスク
    assignByTaskSimilarity(M, N);
  }
#if 0  // debug
  rep(i, N) {
    printf("#task%d was done by member%din%lfdays\n", i,
           completeTimeAndMember[i].first, completeTimeAndMember[i].second);
  }
#endif
}
/*


*/

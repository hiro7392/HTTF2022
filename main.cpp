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
#define debugWorkTimeMax 0
#define debugWorkTimeAverage 1
#define useSample 0
#define useTestDate 0
vector<vector<int>> from(1010);
vector<vector<int>> to(1010);

// skills[i][k]=メンバーi のスキルk
long long skills[25][25];

// timeToNeed[i][k]:=メンバーiがタスクkを終了するまでにかかる日数。
long long timeToNeed[25][1010];

// assing[i]:=メンバーiに割り振られたタスク、タスクがなければ-1を入れる
vector<long long> taskAssign(25);

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

bool cmp(pair<double, int> p1, pair<double, int> p2) {
  return p1.first < p2.first;
}

void makeSelectOrder(int m) {
  rep(i, m) selectOrder[i] = make_pair(0, i);
  return;
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
  long long taskSkill[N][K];
  rep(i, N) {
    rep(l, K) { cin >> taskSkill[i][l]; }
  }
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

  //かかった時間を記録
  rep(k, 25) {
    if (selectOrder[k].second == i) {
      //今までの作業にかかった時間の最大値で更新
      // selectOrder[k].first = max(selectOrder[k].first, workingTime[i]);

      //今までの作業にかかった時間の平均値で更新
      selectOrder[k].first =
          (double)(selectOrder[k].first * completedTask[i] + workingTime[i]) /
          (completedTask[i] + 1.0);
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
      onGoing[k] = true;
      timeUntilFinish[i] = timeToNeed[i][k];
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
  rep(i, m) {
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
void assignByFast(int m, int n) {
  vector<pair<int, int>> output;

  //コストが軽い順にソート
  sort(selectOrder.begin(), selectOrder.begin() + m, cmp);

  rep(i, m) {
    int number = selectOrder[i].second;
    if ((taskAssign[number] < 0) && getTask(number, n)) {
      output.push_back(make_pair(number, taskAssign[number]));
    }
  }
#if debugWorkTime
  rep(i, m) {
    int index = selectOrder[i].second;
#if debugWorkTimeMax
    printf("#s メンバー　%d の コストのは　%dです\n", index,
           selectOrder[i].first);
#elif debugWorkTimeAverage
    printf("#s メンバー　%d の コストのは　%lfです\n", index,
           selectOrder[i].first);
#endif
  }
#endif
  //出力用
  printf("%d ", output.size());
  for (auto now : output) {
    printf("%d %d ", now.first + 1, now.second + 1);
  }
  printf("\n");
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

int main() {
  long long N, M, K, R;
  cin >> N >> M >> K >> R;

  //タスク関係の入力
  makeTasks(N, K);
  //依存関係の入力と構築
  makeDependency(R);
  //メンバーを整列させて初期化
  makeSelectOrder(M);

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
    printf("now status =%d\n", status);
#endif
    assignByFast(M, N);
    // simpleAssign(M, N);

#if debug
    printNowAssignMent(M);
#endif
  }
}
/*


*/

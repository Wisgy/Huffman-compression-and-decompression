#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define SET_BYTE(vbyte, index) ((vbyte) |= (1 << ((index) ^ 7)))
#define CLR_BYTE(vbyte, index) ((vbyte) &= (~(1 << ((index) ^ 7))))
typedef struct ascii_freq {
    unsigned char ascii;
    int freq;
}ascii_freq;
typedef struct huffman_tree{
    struct huffman_tree *parent,*lchild,*rchild;
    int weight;
}huffT;
typedef struct huffman_code{
    unsigned char ascii;
    char *code;
}huffC;
//统计文件字符频度
ascii_freq *count_ascii(char*filename,int *n){
    int fre[256]={0};
    FILE *fp=fopen(filename,"rb");
    unsigned char ch;
    while (!feof(fp)){
        fread(&ch,1,1,fp);
        fre[ch]++;
    }
    for(int i=0;i<256;i++)if(fre[i]!=0)(*n)++;
    int tmp=*n;
    ascii_freq *frequency=malloc(sizeof(ascii_freq)*tmp);
    tmp--;
    for(int i=0;i<256;i++){
        if(fre[i]!=0){frequency[tmp].ascii=i;
        frequency[tmp--].freq=fre[i];}
    }
    fclose(fp);
    return frequency;
}
//构建哈夫曼树
void search_node(huffT*HT,int n,int *s1,int*s2){//最小权重为*s1,次小权重为*s2
    *s1=-1;
    *s2=-1;
    for(int i=0;i<n;i++){
        if(HT[i].parent!=NULL)continue;
        else if(*s1==-1)*s1=i;
        else if(*s2==-1)if(HT[i].weight>=HT[*s1].weight)*s2=i;
            else {*s2=*s1;*s1=i;}
        else if(HT[*s2].weight>HT[i].weight)if(HT[*s1].weight>HT[i].weight){*s2=*s1;*s1=i;}
            else *s2=i;
    }
}
huffT *cre_huffT(ascii_freq*frequency,int n){
    int m=2*n-1;
    huffT*HT=malloc(sizeof(huffT)*m);
    int i=0;
    for(;i<n;i++){//initialize:1~n
        HT[i].weight=frequency->freq;
        HT[i].parent=NULL;
        HT[i].lchild=NULL;
        HT[i].rchild=NULL;
        frequency++;
    }
    for(;i<m;i++){//initialize:n+1~2n-1
        HT[i].weight=0;
        HT[i].parent=NULL;
        HT[i].lchild=NULL;
        HT[i].rchild=NULL;
        frequency++;
    }
    int *s1_p,*s2_p;
    int s1,s2;
    s1_p=&s1;
    s2_p=&s2;
    for(i=n;i<m;i++){
        search_node(HT,i,s1_p,s2_p);//find the two least node in 0~i-1
        HT[s1].parent=&HT[i];
        HT[s2].parent=&HT[i];
        HT[i].lchild=&HT[s1];
        HT[i].rchild=&HT[s2];
        HT[i].weight=HT[s1].weight+HT[s2].weight;
    }
    return HT;
}
//获得编码
huffC*cre_code(huffT*HT,int n,ascii_freq *frequency){
    huffC *HC=malloc(sizeof(huffC)*n);
    char *cd=malloc(sizeof(char)*n);
    cd[n-1]='\0';
    int start;
    huffT*f,*child;
    for(int i=0;i<n;++i){
        start=n-1;
        child=&HT[i];
        for(f=HT[i].parent;f!=NULL;f=f->parent){
            if(f->lchild==child)cd[--start]='0';
            else cd[--start]='1';
            child=f;
        }
        HC[i].ascii=frequency[i].ascii;
        HC[i].code=malloc(sizeof(char)*(n-start));
        strcpy(HC[i].code,&cd[start]);
    }
    free(cd);
    return HC;
}
//将字符对应编码写入文件
int conti_huf(char *originname,char*newname,int *huf_ind,huffC*HC){
    FILE*fpIn=fopen(originname,"rb");
    FILE*fpOut=fopen(newname,"ab");
    unsigned char ch;
    fread(&ch,1,1,fpIn);
    unsigned char value;
    int index=0;
    int i;
	while(!feof(fpIn)) {
		char *hufCode = HC[huf_ind[ch]].code;//将code写入value字节中
		for(i = 0; hufCode[i]!='\0'; i++) {
			if('0' == hufCode[i]) {
				//从第1位依次赋值，若大于八位（一个字节）了，就写入文件中
				CLR_BYTE(value, index);
			} else {
				SET_BYTE(value, index);
			}
			index++;
			if(index >= 8) {
				index = 0;
				fwrite(&value, sizeof(unsigned char), 1, fpOut);
			}
		}
		fread(&ch,1,1,fpIn);
	}
	//如果最后一次不满一个字节，依然需要写到文件中，注意：写入的最后一个字节可能会存在垃圾位
	if(index) {
		fwrite(&value, sizeof(unsigned char), 1, fpOut);
	}
    fclose(fpIn);
    fclose(fpOut);
    return 0;
}
//绘制哈夫曼表写入新文件头部
void wb_huff(char*filename,huffC*HC,int n,ascii_freq*frequency){
    char *newname;
    char *cd=malloc(sizeof(char)*50);
    char *suffix=malloc(sizeof(char)*6);
    int i,j;
    for(i=0;filename[i]!='.';i++)cd[i]=filename[i];
    for(j=0;filename[j+i]!='\0';j++)suffix[j]=filename[j+i+1];
    cd[i++]='.';
    cd[i++]='s';
    cd[i++]='c';
    cd[i++]='y';
    cd[i]='\0';
    strcpy(newname,cd);
    free(cd);
    FILE*f=fopen(newname,"wb");
    //写入文件格式
    fwrite(suffix,sizeof(char),j,f);
    //写入huffmancode
    fwrite(&n,sizeof(int),1,f);
    fwrite(frequency,sizeof(ascii_freq),n,f);
    fclose(f);
    //构建ascii码与索引对应的哈希表
    int *huf_ind=malloc(sizeof(int)*256);
    for(int k=0;k<256;k++)huf_ind[k]=-1;
    for(int k=0;k<n;k++)huf_ind[HC[k].ascii]=k;
    conti_huf(filename,newname,huf_ind,HC);
}
//主函数
int main(){
    int *n_pointer;
    int n=0;
    n_pointer=&n;
    char *filename="testdir.a";
    ascii_freq*frequency=count_ascii(filename,n_pointer);
    huffT*HT=cre_huffT(frequency,n);
    huffC*HC=cre_code(HT,n,frequency);
    wb_huff(filename,HC,n,frequency);
    return 0;
}
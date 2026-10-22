"""Translate the reviewed script AST into the supported remake operations."""
OP_NAMES = ['Stop', 'Text', 'EventText', 'SetEvent', 'ClearEvent', 'IfEvent',
            'IfPosition', 'IfYes', 'IfTeam', 'IfDead', 'IfLevel', 'IfMoney',
            'IfItem', 'IfTreasure', 'Jump', 'Loop', 'Wait', 'WaitSet', 'WaitClear',
            'Face', 'Move', 'MoveTo', 'Teleport', 'Wander', 'Chase', 'Animate',
            'Hide', 'Recover', 'Spend', 'Recruit', 'Battle', 'ChangeModel', 'SetFrame',
            'FacePlayer','FaceBack','Offset','IfArea','IfFace','SetAttrs','SpriteBank',
            'ModelDelta','Control','Music','Menu','PlayerShow','PlayerHide','PlayerHideAll',
            'PlayerBecome','PlayerTeleport','PlayerFace','PlayerPull','PlayerUnpull',
            'Scroll','Scene','SceneEnd','Quake','Flash','Sleep','NextDay','Code',
            'DrawTile','OpenDoor','NpcPlay','NpcPatrol','NpcEnterTank','NpcExitTank',
            'NpcBecome','NpcThrow','NpcExplode','NpcHurt','MoveTank','IfDetector',
            'IfHp','IfDrank','IfRideTank','IfTankRiding','IfTankHere','IfHasOkTank',
            'GiveItem','SwapItem','TeamJoin','TeamLeave','TeamHide','System','Conveyor',
            'ChangeTankMap','Medal','Henshin','Unsupported']

def compile_script(ast, types, unsupported):
    nodes=[];labels={};fixups=[]
    def emit(op,a=0,b=0,c=0,d=0,target=-1):nodes.append([types.index(op),a,b,c,d,target]);return len(nodes)-1
    def block(seq):
        for n in seq:
            if n.get('label'):labels[n['label']]=len(nodes)
            kind=n['$type'];op=n.get('op','');kids=n.get('scripts',[])
            cond={'Event':('IfEvent','eventCode'),'Xy':('IfPosition','x'),'Yes':('IfYes',None),'Team':('IfTeam','player'),'TeammateDead':('IfDead','teammate'),'Level':('IfLevel','level'),'Money':('IfMoney','money'),'HasItem':('IfItem','item'),'Treasure':('IfTreasure','treasure'),'Area':('IfArea','XX1'),'Face':('IfFace','face'),'DetectorScript':('IfDetector','range'),'Hp':('IfHp','hp'),'Drank':('IfDrank',None),'RideTank':('IfRideTank',None),'Riding':('IfTankRiding','tank'),'Here':('IfTankHere','tank'),'HasOkTank':('IfHasOkTank',None)}
            if kind=='Event' and 'trueLine' in n:emit('EventText',n['eventCode'],n['page'],n['trueLine'],n['falseLine']);continue
            if kind=='Event' and op=='62':emit('WaitClear',n['eventCode']);continue
            if kind in cond and ('isLoopList' in n or 'scripts' in n):
                name,key=cond[kind]
                if kind=='Area':
                    args=(n['XX1'],n['XX2'],n['YY1'],(n['YY2']&127)|(128 if n.get('isLoopList',False) else 0))
                elif kind=='Face':
                    face=n.get('face',0)
                    if isinstance(face,str):
                        named={'up':0,'down':1,'left':2,'right':3}
                        face=named[face] if face in named else int(face,16)
                    args=(face,0,int(n.get('isLoopList',False)),0)
                else:
                    args=(n.get(key,0) if key else 0,n.get('y',0),int(n.get('isLoopList',False)),0)
                idx=emit(name,args[0],args[1],args[2],args[3])
                if n.get('targetLabel'):fixups.append((idx,n['targetLabel']))
                else:
                    block(kids)
                    if n.get('isLoopList'):emit('Jump',target=idx)
                    nodes[idx][5]=len(nodes)
                continue
            if kind=='LoopScript':
                begin=len(nodes);block(kids);emit('Loop',n.get('count',0),target=begin);continue
            if kind=='JmpScript':idx=emit('Jump');fixups.append((idx,n['targetLabel']));continue
            if kind=='Quote':emit('Text',n['page'],n['line']);continue
            if kind=='EndScript':emit('Stop');continue
            if kind in ('Open','Close'):emit('SetEvent' if kind=='Open' else 'ClearEvent',n['eventCode']);continue
            if kind=='Time':emit('Wait',n['time']);continue
            if kind=='Wait':emit('WaitSet',n['eventCode']);continue
            if kind in ('Up','Down','Left','Right'):
                if op in ('0C','0D','0E','0F'):emit('Face', ['Up','Down','Left','Right'].index(kind))
                elif op in ('12','13','14','15'):emit('Move',['Up','Down','Left','Right'].index(kind),n.get('count',1))
                elif op in ('50','51','52','53'):emit('Scroll',['Up','Down','Left','Right'].index(kind),n.get('count',1))
                else:unsupported[kind+':'+op]+=1;emit('Unsupported',int(op or 'ff',16))
                continue
            if kind=='To':emit('MoveTo',n['x'],n['y']);continue
            if kind=='Tp':emit('Teleport',n['x'],n['y']);continue
            if kind=='TpOffset':emit('Offset',n['xy']);continue
            if kind=='Wander':emit('Wander',n.get('x1',-1),n.get('y1',-1),n.get('x2',-1),n.get('y2',-1));continue
            if kind=='Chase':emit('Chase');continue
            if kind=='Act':emit('Animate');continue
            if kind=='Play':emit('NpcPlay',n['anim']);continue
            if kind in ('TileType','Set'):emit('ChangeModel',n['model'],int(kind=='Set'));continue
            if kind=='Frame':emit('SetFrame',n['frame']);continue
            if kind=='Attrs':emit('SetAttrs',n['attrs']);continue
            if kind=='Sprite':emit('SpriteBank',n['bank']);continue
            if kind=='Next':emit('ModelDelta',1);continue
            if kind=='Previous':emit('ModelDelta',-1);continue
            if kind=='Resume':emit('Animate');continue
            if kind=='Player' and op=='11':emit('FacePlayer');continue
            if kind=='Back' and op=='10':emit('FaceBack');continue
            if kind=='HideScript':emit('Hide');continue
            if kind=='RemoveScript':emit('Hide');emit('Stop');continue
            if kind=='RecoverScript':emit('Recover',n.get('player',0));continue
            if kind=='Spend':emit('Spend',n['money']);continue
            if kind=='Now':emit('Recruit',n['player']);continue
            if kind=='BattleScript':emit('Battle',n['monster'],n.get('eventCode',0),n.get('stage',0));continue
            if kind=='SpeakerScript':continue
            if kind=='Change':emit('Control');continue
            if kind=='MusicScript':emit('Music',n['music']);continue
            if kind=='MenuScript':emit('Menu',n['fun'],n['line']);continue
            if kind=='Show':emit('PlayerShow',n.get('custom',0));continue
            if kind=='All':emit('PlayerHideAll');continue
            if kind=='Become' and op=='4E':emit('PlayerBecome',n['player']);continue
            if kind=='Become' and op=='40':emit('NpcBecome',n['player']);continue
            if kind=='Face' and op=='6D':emit('PlayerFace',int(n.get('direction','0'),16));continue
            if kind=='Pull':emit('PlayerPull',n['pull']);continue
            if kind=='Unpull':emit('PlayerUnpull');continue
            if kind=='Tp' and op in ('','4C'):emit('PlayerTeleport',n.get('map',255),n.get('x',255),n.get('y',255),int(n.get('quiet',False)));continue
            if kind=='SceneScript':emit('Scene',n['stage']);continue
            if kind=='End':emit('SceneEnd');continue
            if kind=='QuakeScript':emit('Quake',n['time']);continue
            if kind=='FlashScreenScript':emit('Flash',n['time']);continue
            if kind=='SleepScript':emit('Sleep',n['room']);continue
            if kind=='NextDayScript':emit('NextDay');continue
            if kind=='CodeScript':emit('Code',int(n.get('codes','ff'),16));continue
            if kind=='DrawTile':emit('DrawTile',n['tile']);continue
            if kind=='OpenDoor':emit('OpenDoor');continue
            if kind=='Patrol':emit('NpcPatrol',n['type']);continue
            if kind=='EnterTank':emit('NpcEnterTank');continue
            if kind=='ExitTank':emit('NpcExitTank',n['model']);continue
            if kind=='Throw':emit('NpcThrow',n['b1'],n['npc']);continue
            if kind=='Explode':emit('NpcExplode');continue
            if kind=='Hurt':emit('NpcHurt');continue
            if kind=='Tank':emit('MoveTank');continue
            if kind=='Item' and op=='49':emit('GiveItem',n['item'],n['text']);continue
            if kind=='Item' and op=='3A':emit('SwapItem',n['B1'],n['B2']);continue
            if kind=='Join':emit('TeamJoin',n['player']);continue
            if kind=='Leave':emit('TeamLeave',n['player']);continue
            if kind=='Hide' and op=='5F':emit('TeamHide');continue
            if kind=='SystemScript':emit('System',n['commandCode']);continue
            if kind=='ConveyorScript':emit('Conveyor',int(n['dir'],16));continue
            if kind=='ChangeTankMapScript':emit('ChangeTankMap',n['fromMap'],n['toMap']);continue
            if kind=='MedalScript':emit('Medal');continue
            if kind=='HenshinScript':emit('Henshin');continue
            # Unsupported effects are a barrier: never execute later story
            # side effects after silently skipping an unknown instruction.
            unsupported[kind+':'+op]+=1;emit('Unsupported',int(op or 'ff',16))
    block(ast);emit('Stop')
    for idx,label in fixups:
        if label in labels:nodes[idx][5]=labels[label]
        else:nodes[idx][0]=types.index('Unsupported')
    return nodes

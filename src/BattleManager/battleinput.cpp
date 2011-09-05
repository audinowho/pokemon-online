#include "battleinput.h"
#include <memory>
#include "../PokemonInfo/battlestructs.h"

typedef std::shared_ptr<ShallowBattlePoke> shallowpoke;

void BattleInput::receiveData(QByteArray inf)
{
    if (delayed && inf[0] != char(BattleChat) && inf[0] != char(SpectatorChat) && inf[0] != char(ClockStart) && inf[0] != char(ClockStop)
            && inf[0] != char(Spectating)) {
        delayedCommands.push_back(inf);
        return;
    }

    QDataStream in (&inf, QIODevice::ReadOnly);
    in.setVersion(QDataStream::Qt_4_6);

    uchar command;
    qint8 player;

    in >> command >> player;

    dealWithCommandInfo(in, command, player);
}

void BattleInput::dealWithCommandInfo(QDataStream &in, uchar command, int spot)
{
    switch (command)
    {
    case SendOut:
    {
        bool silent;
        quint8 prevIndex;
        shallowpoke poke (new ShallowBattlePoke());
        in >> silent;
        in >> prevIndex;
        in >> *poke;
        output<BattleEnum::SendOut>(spot, silent, prevIndex, &poke);
        break;
    }
    case SendBack:
    {
        output<BattleEnum::SendBack>(spot);
        break;
    }
    case UseAttack:
    {
        qint16 attack;
        in >> attack;
        output<BattleEnum::UseAttack>(spot, attack);
        break;
    }
    case BeginTurn:
    {
        quint16 turn;
        in >> turn;
        output<BattleEnum::Turn>(turn);
        break;
    }
    case ChangeHp:
    {
        quint16 newHp;
        in >> newHp;
        output<BattleEnum::NewHp>(spot, newHp);
        break;
    }
    case Ko:
        output<BattleEnum::Ko>(spot);
        break;
    case Hit:
    {
        quint8 number;
        in >> number;
        output<BattleEnum::Hits>(number);
        break;
    }
    case Effective:
    {
        quint8 eff;
        in >> eff;

        output<BattleEnum::EffectiveNess>(spot, eff);
        break;
    }
    case CriticalHit:
        output<BattleEnum::CriticalHit>();
        break;
    case Miss:
    {
        output<BattleEnum::Miss>(spot);
        break;
    }
    case Avoid:
    {
        output<BattleEnum::Avoid>(spot);
        break;
    }
    case StatChange:
    {
        qint8 stat, boost;
        in >> stat >> boost;
        output<BattleEnum::StatChange>(spot, stat, boost);
        break;
    }
    case StatusChange:
    {
        qint8 status;
        in >> status;
        bool multipleTurns;
        in >> multipleTurns;

        output<BattleEnum::ClassicStatusChange>(spot, status);
        break;
    }
    case AbsStatusChange:
    {
        qint8 poke, status;
        in >> poke >> status;

        if (poke < 0 || poke >= 6)
            break;

        output<BattleEnum::AbsoluteStatusChange>(spot,poke,status);
        break;
    }
    case AlreadyStatusMessage:
    {
        quint8 status;
        in >> status;
        output<BattleEnum::AlreadyStatusMessage>(spot,status);
        break;
    }
    case StatusMessage:
    {
        qint8 status;
        in >> status;
        output<BattleEnum::StatusMessage>(spot, status);
        break;
    }
    case Failed:
    {
        output<BattleEnum::Fail>(spot);
        break;
    }
    case BattleChat:
    case EndMessage:
    {
        QString message;
        in >> message;
        output<BattleEnum::PlayerMessage>(spot, message.toUtf8().constData());
        break;
    }
    case Spectating:
    {
        bool come;
        qint32 id;
        in >> come >> id;
        output<BattleEnum::Spectator>(come, id);
        break;
    }
    case SpectatorChat:
    {
        qint32 id;
        QString message;
        output<BattleEnum::SpectatorMessage>(id, message.toUtf8().constData());
        break;
    }
    case MoveMessage:
    {
        quint16 move=0;
        uchar part=0;
        qint8 type(0), foe(0);
        qint16 other(0);
        QString q;
        in >> move >> part >> type >> foe >> other >> q;
        output<BattleEnum::MoveMessage>(spot, move, part, type, foe, other, q.toUtf8().constData());
        break;
    }
    case NoOpponent:
        output<BattleEnum::NoTargetMessage>(spot);
        break;
    case ItemMessage:
    {
        quint16 item=0;
        uchar part=0;
        qint8 foe = 0;
        qint16 other=0;
        qint16 berry = 0;
        in >> item >> part >> foe >> berry >> other;
        output<BattleEnum::ItemMessage>(spot, item, part, foe, berry, other);
        break;
    }
    case Flinch:
    {
        output<BattleEnum::Flinch>(spot);
        break;
    }
    case Recoil:
    {
        bool damage;
        in >> damage;

        if (damage)
            output<BattleEnum::Recoil>(spot);
        else
            output<BattleEnum::Drained>(spot);
        break;
    }
    case WeatherMessage: {
        qint8 wstatus, weather;
        in >> wstatus >> weather;
        if (weather == NormalWeather)
            break;

        switch(wstatus) {
        case EndWeather:
            output<BattleEnum::EndWeather>(weather);
            break;
        case HurtWeather:
            output<BattleEnum::WeatherDamage>(spot, weather);
            break;
        case ContinueWeather:
            output<BattleEnum::WeatherMessage>(weather);
            break;
        }
    } break;
    case StraightDamage :
    {
        qint16 damage;
        in >> damage;

        output<BattleEnum::Damaged>(spot, damage);
        break;
    }
    case AbilityMessage:
    {
        quint16 ab=0;
        uchar part=0;
        qint8 type(0), foe(0);
        qint16 other(0);
        in >> ab >> part >> type >> foe >> other;
        output<BattleEnum::AbilityMessage>(spot, ab, part, type, foe, other);
        break;
    }
    case Substitute:
    {
        bool sub;
        in >> sub;
        output<BattleEnum::SubstituteStatus>(spot, sub);
        break;
    }
    case BattleEnd:
    {
        qint8 res;
        in >> res;
        output<BattleEnum::BattleEnd>(spot, res);
        break;
    }
    case BlankMessage: {
        output<BattleEnum::BlankMessage>();
        break;
    }
    case Clause:
    {
        output<BattleEnum::ClauseMessage>(spot);
        break;
    }
    case Rated:
    {
        bool rated;
        in >> rated;

        output<BattleEnum::RatedInfo>(rated);
        break;
    }
    case TierSection:
    {
        QString tier;
        in >> tier;
        output<BattleEnum::TierInfo>(tier.toUtf8().constData());
        break;
    }
    case DynamicInfo:
    {
        BattleDynamicInfo info;
        in >> info;
        output<BattleEnum::StatBoostsAndField>(info);
        break;
    }
    case TempPokeChange:
    {
        quint8 type;
        in >> type;
        if (type == TempSprite) {
            Pokemon::uniqueId tempsprite;
            in >> tempsprite;

            if (tempsprite == -1) {
                output<BattleEnum::PokemonVanish>(spot);
            } else if (tempsprite == Pokemon::NoPoke) {
                output<BattleEnum::PokemonReappear>(spot);
            } else {
                output<BattleEnum::SpriteChange>(spot, tempsprite.toPokeRef());
            }
        } else if (type == DefiniteForme)
        {
            quint8 poke;
            Pokemon::uniqueId newforme;
            in >> poke >> newforme;
            output<BattleEnum::DefiniteFormeChange>(spot, poke, newforme.toPokeRef());
        } else if (type == AestheticForme)
        {
            quint16 newforme;
            in >> newforme;
            output<BattleEnum::CosmeticFormeChange>(spot, newforme);
        }
        break;
    }
    case ClockStart:
    {
        quint32 time;
        output<BattleEnum::ClockStart>(spot, time);
        break;
    }
    case ClockStop:
    {
        quint32 time;
        output<BattleEnum::ClockStop>(spot, time);
        break;
    }
    case SpotShifts:
    {
        qint8 s1, s2;
        bool silent;

        in >> s1 >> s2 >> silent;

        output<BattleEnum::ShiftSpots>(s1, s1, silent);
        break;
    }
    default:
        /* TODO: UNKNOWN COMMAND */
        break;
    }
}
